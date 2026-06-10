#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/JsonUtils.h"

Engine::Engine()
    : m_serialChannel(m_dispatcher),
      m_tcpChannel(m_dispatcher, COMMAND_PORT, RESPONSE_PORT),
      m_bleChannel(m_dispatcher, DEFAULT_WIFI_SSID),
      m_udpHeartbeatSink(m_wifi, HEARTBEAT_PORT),
      m_udpReportSink(m_wifi, REPORT_PORT),
      m_bleSink(m_bleChannel.transport()),
      m_describeHandler(m_config, m_fixtures, m_inputs, m_wifi),
      m_getFixtureHandler(m_fixtures),
      m_setFixtureHandler(m_fixtures),
      m_inputsHandler(m_inputs),
      m_wifiConfigHandler(m_wifi, m_config, m_storage),
      m_engineConfigHandler(m_config, m_storage, m_publisher),
      m_fixtures(m_inputs)
{
    Utils::Logger::enable();
    Utils::Logger::setLevel(Utils::Logger::DEBUG);
}

void Engine::init()
{
    Utils::Logger::println("Storage");
    m_storage.setPartition("engine");
    Utils::Logger::println("Config");

    loadConfig();
    Utils::Logger::println("Wifi");

    m_wifi.connect(m_config.ssid, m_config.password);

    Utils::Logger::println("Register");

    registerCommands();

    // BLE config channel comes up independent of wifi, so credentials can be
    // set over Bluetooth before the device has ever joined a network.
    m_serialChannel.begin();
    m_bleChannel.begin();

    Utils::Logger::println("Spaw3n");

    spawnTasks();

    Utils::Logger::printf("[ENGINE] v%s initialized\n", m_config.version.c_str());
}

std::string Engine::toString()
{
    JsonDocument doc;
    // doc["heap_size"] = ESP.getHeapSize();
	// doc["heap_free"] = ESP.getFreeHeap();
    JsonObject settingsDoc = doc["settings"].to<JsonObject>();
    m_config.toJson(settingsDoc);

    JsonObject wifiDoc = doc["wifi"].to<JsonObject>();
    m_wifi.toJson(wifiDoc);

    JsonObject fixtureDoc = doc["fixture_handler"].to<JsonObject>();
    m_fixtures.toJson(fixtureDoc);

    JsonObject inputDoc = doc["input_handler"].to<JsonObject>();
    m_inputs.toJson(inputDoc);

    std::string temp;
    serializeJson(doc, temp);
    return temp;
}

void Engine::loadConfig()
{
    if (m_storage.isKey("settings"))
    {
        std::string s = m_storage.getString("settings");
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, s);
        if(err)
        {
            Utils::Logger::println("[ERROR] Engine: read settings json");
            return;
        }
        m_factoryReset = Utils::Json::getElement<bool>(doc, "to_factory_settings", false);
        if(m_factoryReset)
        {
            m_config.to_factory_settings = false;
            m_storage.putString("settings", m_config.toString());
            s = m_storage.getString("settings");
            deserializeJson(doc, s);
        }
        Utils::Json::updateElement<bool>(doc, "serial_report_task", m_config.serial_report_task);
        Utils::Json::updateElement<bool>(doc, "wireless_report_task", m_config.wireless_report_task);
        Utils::Json::updateElement<bool>(doc, "wifi_animation", m_config.wifi_animation);

        const char* ssid = Utils::Json::getElement<const char*>(doc, "ssid", m_config.ssid.c_str());
        m_config.ssid = ssid;
        const char* password = Utils::Json::getElement<const char*>(doc, "password", m_config.password.c_str());
        m_config.password = password;
    }
    else
    {
        saveConfig();
    }
}

void Engine::saveConfig()
{
    m_storage.putString("settings", m_config.toString());
}

void Engine::registerCommands()
{
    m_dispatcher.registerHandler("describe", &m_describeHandler);
    m_dispatcher.registerHandler("fixtures", &m_getFixtureHandler);
    m_dispatcher.registerHandler("getfixture", &m_getFixtureHandler);
    m_dispatcher.registerHandler("setfixture", &m_setFixtureHandler);
    m_dispatcher.registerHandler("inputs", &m_inputsHandler);
    m_dispatcher.registerHandler("wifi", &m_wifiConfigHandler);
    m_dispatcher.registerHandler("reboot", &m_rebootHandler);
    m_dispatcher.registerHandler("factory_reset", &m_engineConfigHandler);
    m_dispatcher.registerHandler("serial_report_task", &m_engineConfigHandler);
    m_dispatcher.registerHandler("wireless_report_task", &m_engineConfigHandler);
    m_dispatcher.registerHandler("wifi_animation", &m_engineConfigHandler);
}

void Engine::onWifiStatus(bool connected)
{
    if (connected)
    {
        m_tasks.suspendTask(m_animationTaskId);
        m_tasks.resumeTask(m_fixtureTaskId);

        m_tcpChannel.begin();
        m_udpHeartbeatSink.begin();
        m_udpReportSink.begin();
        m_inputs.reconnect();
    }
    else
    {
        m_tasks.suspendTask(m_fixtureTaskId);
        if (m_config.wifi_animation)
        {
            m_tasks.resumeTask(m_animationTaskId);
        }

        m_tcpChannel.stop();
    }
}

void Engine::spawnTasks()
{
    // Input polling task
    m_tasks.spawnTask("input_update", [this]() {
        while (true)
        {
            m_inputs.update();
            vTaskDelay(1);
        }
    }, 3, 30000);

    // Fixture update task — starts suspended, resumed on wifi connect
    m_fixtureTaskId = m_tasks.spawnTask("fixture_update", [this]() {
        while (true)
        {
            m_fixtures.update();
            vTaskDelay(1);
        }
    }, 2, 30000);
    m_tasks.suspendTask(m_fixtureTaskId);

    // Wifi animation task — runs until first wifi connect
    m_animationTaskId = m_tasks.spawnTask("wifi_animation", [this]() {
        while (true)
        {
            m_fixtures.wifiAnimation();
            vTaskDelay(20);
        }
    }, 1, 4096);

    // Wifi monitor task
    m_tasks.spawnTask("wifi_monitor", [this]() {
        bool wasConnected = false;
        while (true)
        {
            bool connected = m_wifi.isConnected();
            if (connected != wasConnected)
            {
                onWifiStatus(connected);
                wasConnected = connected;
            }
            vTaskDelay(50);
        }
    }, 1, 3000);

    // Outbound periodic broadcasts (heartbeat + reports) run on one task.
    registerReports();
    m_tasks.spawnTask("publisher", [this]() {
        while (true)
        {
            m_publisher.tick();
            vTaskDelay(10);
        }
    }, 1, 16000);

    // Command channels — each polled on its own task so a slow read on one
    // channel never stalls the others. All feed the same CommandDispatcher.
    m_tasks.spawnTask("serial_cmd", [this]() {
        while (true)
        {
            m_serialChannel.poll();
            vTaskDelay(10);
        }
    }, 1, 8192);

    // TCP command channel — receive on COMMAND_PORT, respond on RESPONSE_PORT
    m_tasks.spawnTask("tcp_cmd", [this]() {
        while (true)
        {
            m_tcpChannel.poll();
            vTaskDelay(10);
        }
    }, 2, 8192);

    // BLE command channel — config over Bluetooth, available without wifi
    m_tasks.spawnTask("ble_cmd", [this]() {
        while (true)
        {
            m_bleChannel.poll();
            vTaskDelay(10);
        }
    }, 1, 8192);
}

void Engine::registerReports()
{
    // Heartbeat: small fixture summary broadcast over UDP every 500ms.
    m_publisher.add({ "heartbeat",
        [this]() { return heartbeatJson(); },
        &m_udpHeartbeatSink, 500 });

    // Wireless report: full settings dump over UDP, toggled via config.
    m_publisher.add({ Commands::WIRELESS_REPORT_FEED,
        [this]() { return m_config.toString(); },
        &m_udpReportSink, 10000, m_config.wireless_report_task });

    // Serial report: full engine state printed to the console, toggled via config.
    m_publisher.add({ Commands::SERIAL_REPORT_FEED,
        [this]() { return toString(); },
        &m_serialSink, 10000, m_config.serial_report_task });

    // BLE report: same engine state pushed to a connected central. Opt-in —
    // shares the NUS TX characteristic with command responses, so enable it
    // only when a client expects an unsolicited feed.
    m_publisher.add({ "ble_report",
        [this]() { return toString(); },
        &m_bleSink, 10000, false });
}

std::string Engine::heartbeatJson()
{
    if (m_heartbeatCache.empty())
    {
        JsonDocument doc;
        doc["version"] = m_config.version;
        JsonArray arr = doc["fixtures"].to<JsonArray>();
        for (auto fix : m_fixtures.all())
        {
            JsonObject entry = arr.add<JsonObject>();
            entry["name"] = fix->getName();
            entry["type"] = fix->getType();
        }
        serializeJson(doc, m_heartbeatCache);
    }
    return m_heartbeatCache;
}
