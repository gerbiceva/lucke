#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/JsonUtils.h"

Engine::Engine()
    : m_tcpServer(COMMAND_PORT),
      m_fixtures(m_inputs),
      m_statusReport(m_config, m_wifi, m_fixtures, m_inputs),
      m_telemetry(m_config, m_wifi, m_fixtures, m_statusReport, m_tasks),
      m_describeHandler(m_config, m_fixtures, m_inputs, m_wifi),
      m_getFixtureHandler(m_fixtures),
      m_setFixtureHandler(m_fixtures),
      m_inputsHandler(m_inputs),
      m_wifiConfigHandler(m_wifi, m_config, m_storage),
      m_engineConfigHandler(m_config, m_storage, m_telemetry)
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

        m_tcpServer.begin();
        m_inputs.reconnect();
    }
    else
    {
        m_tasks.suspendTask(m_fixtureTaskId);
        if (m_config.wifi_animation)
        {
            m_tasks.resumeTask(m_animationTaskId);
        }

        m_tcpServer.stop();
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

    // Serial command task
    m_tasks.spawnTask("serial_cmd", [this]() {
        while (true)
        {
            if (m_serial.isAvailable())
            {
                std::string line = m_serial.readLine();
                m_dispatcher.dispatch(line, m_serial);
            }
            vTaskDelay(10);
        }
    }, 1, 8192);

    // BLE command task — same JSON command protocol as serial/TCP, carried
    // over the Nordic UART Service characteristics.
    // m_tasks.spawnTask("ble_config", [this]() {
    //     while (true)
    //     {
    //         if (m_ble.isAvailable())
    //         {
    //             std::string line = m_ble.readLine();
    //             m_dispatcher.dispatch(line, m_ble);
    //         }
    //         vTaskDelay(10);
    //     }
    // }, 1, 8192);

    // TCP command task — receive on 8888, respond on RESPONSE_PORT
    m_tasks.spawnTask("tcp_cmd", [this]() {
        while (true)
        {
            auto conn = m_tcpServer.accept();
            if (conn.has_value())
            {
                auto& tc = conn.value();
                std::string remoteIp = tc.remoteAddress();
                while (tc.isConnected())
                {
                    if (tc.isAvailable())
                    {
                        std::string line = tc.readLine();
                        Net::TcpConnection response(remoteIp, RESPONSE_PORT);
                        m_dispatcher.dispatch(line, response);
                        response.close();
                    }
                    vTaskDelay(10);
                }
            }
            vTaskDelay(10);
        }
    }, 2, 8192);
}
