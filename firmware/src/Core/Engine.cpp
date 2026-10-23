#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/JsonUtils.h"

Engine::Engine()
    : m_fixtures(m_inputs),
      m_statusReport(m_config, m_wifi, m_fixtures, m_inputs),
      m_telemetry(m_config, m_wifi, m_fixtures, m_statusReport, m_tasks),
      m_describeHandler(m_config, m_fixtures, m_inputs, m_wifi),
      m_getFixtureHandler(m_fixtures),
      m_setFixtureHandler(m_fixtures),
      m_inputsHandler(m_inputs),
      m_wifiConfigHandler(m_wifi, m_config, m_storage),
      m_engineConfigHandler(m_config, m_storage, m_telemetry),
      m_serialSession(m_serial, m_dispatcher, m_tasks, "serial_cmd"),
      m_tcpListener(COMMAND_PORT, RESPONSE_PORT, m_dispatcher, m_tasks)
{
    Utils::Logger::enable();
    Utils::Logger::setLevel(Utils::Logger::Level::DEBUG);
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

    // Utils::Logger::println("BLE");
    // m_ble.begin("Lucke");

    Utils::Logger::println("Spawn");
    spawnTasks();

    Utils::Logger::printf("[ENGINE] %.*s initialized\n", static_cast<int>(m_config.version.size()), m_config.version.data());
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

        m_tcpListener.begin();
        m_inputs.reconnect();
    }
    else
    {
        m_tasks.suspendTask(m_fixtureTaskId);
        if (m_config.wifi_animation)
        {
            m_tasks.resumeTask(m_animationTaskId);
        }

        m_tcpListener.stop();
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

    m_telemetry.start();

    m_serialSession.start();
    m_tcpListener.start();
}
