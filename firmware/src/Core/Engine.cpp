#include "Engine.h"
#include "Utils/Logger.h"

Engine::Engine()
    : m_inputs(m_tasks),
      m_fixtures(m_inputs, m_settings),
      m_render(m_fixtures, m_tasks),
      m_connectivity(m_wifi, m_tasks),
      m_statusReport(m_settings, m_wifi, m_fixtures, m_inputs),
      m_telemetry(m_settings.engine(), m_wifi, m_fixtures, m_statusReport, m_tasks),
      m_describeHandler(m_settings, m_fixtures, m_inputs, m_wifi),
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
    startServices();

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

// Everything that reacts to the wifi link. Listeners run on the
// Connectivity task, in this order.
void Engine::wireConnectivity()
{
    m_connectivity.onChange([this](bool connected) {
        using Mode = Services::RenderLoop::Mode;
        if (connected)
            m_render.setMode(Mode::Live);
        else
            m_render.setMode(m_settings.snapshot().wifi_animation ? Mode::Animation : Mode::Off);
    });

    m_connectivity.onChange([this](bool connected) {
        if (connected)
            m_tcpListener.begin();
        else
            m_tcpListener.stop();
    });

    m_connectivity.onChange([this](bool connected) {
        if (connected)
            m_inputs.reconnect();
    });
}

void Engine::startServices()
{
    m_inputs.start();

    // Animation until wifi first connects, then live DMX (see wireConnectivity)
    m_render.start();

    wireConnectivity();
    m_connectivity.start();

    m_telemetry.start();

    m_serialSession.start();
    m_tcpListener.start();
}
