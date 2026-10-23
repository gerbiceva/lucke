#include "Engine.h"
#include "Utils/Logger.h"

Engine::Engine()
    : m_fixtures(m_inputs, m_settings),
      m_render(m_fixtures, m_tasks),
      m_statusReport(m_settings.engine(), m_wifi, m_fixtures, m_inputs),
      m_telemetry(m_settings.engine(), m_wifi, m_fixtures, m_statusReport, m_tasks),
      m_describeHandler(m_settings.engine(), m_fixtures, m_inputs, m_wifi),
      m_getFixtureHandler(m_fixtures),
      m_setFixtureHandler(m_fixtures),
      m_inputsHandler(m_inputs),
      m_wifiConfigHandler(m_wifi, m_settings),
      m_engineConfigHandler(m_settings, m_telemetry),
      m_serialSession(m_serial, m_dispatcher, m_tasks, "serial_cmd"),
      m_tcpListener(COMMAND_PORT, RESPONSE_PORT, m_dispatcher, m_tasks)
{
    Utils::Logger::enable();
    Utils::Logger::setLevel(Utils::Logger::Level::DEBUG);
}

void Engine::init()
{
    Utils::Logger::println("Settings");
    m_settings.load();
    const Config::Engine& config = m_settings.engine();

    Utils::Logger::println("Wifi");
    m_wifi.connect(config.ssid, config.password);

    Utils::Logger::println("Register");
    registerCommands();

    // Utils::Logger::println("BLE");
    // m_ble.begin("Lucke");

    Utils::Logger::println("Spawn");
    spawnTasks();

    Utils::Logger::printf("[ENGINE] %.*s initialized\n", static_cast<int>(config.version.size()), config.version.data());
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
        m_render.setMode(Services::RenderLoop::Mode::Live);

        m_tcpListener.begin();
        m_inputs.reconnect();
    }
    else
    {
        m_render.setMode(m_settings.engine().wifi_animation
                             ? Services::RenderLoop::Mode::Animation
                             : Services::RenderLoop::Mode::Off);

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

    // Animation until wifi first connects, then live DMX (see onWifiStatus)
    m_render.start();

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
