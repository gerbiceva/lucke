#include "Engine.h"
#include <esp_system.h>
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"ENGINE"};

    void logResetReason()
    {
        const char* name = "unknown";
        bool abnormal = true;
        switch (esp_reset_reason())
        {
            case ESP_RST_POWERON:    name = "power on"; abnormal = false; break;
            case ESP_RST_EXT:        name = "external pin"; abnormal = false; break;
            case ESP_RST_SW:         name = "software restart"; abnormal = false; break;
            case ESP_RST_USB:        name = "USB"; abnormal = false; break;
            case ESP_RST_JTAG:       name = "JTAG"; abnormal = false; break;
            case ESP_RST_DEEPSLEEP:  name = "deep sleep wake"; abnormal = false; break;
            case ESP_RST_PANIC:      name = "crash (panic)"; break;
            case ESP_RST_INT_WDT:    name = "interrupt watchdog"; break;
            case ESP_RST_TASK_WDT:   name = "task watchdog"; break;
            case ESP_RST_WDT:        name = "watchdog"; break;
            case ESP_RST_BROWNOUT:   name = "brownout (supply voltage dropped)"; break;
            case ESP_RST_PWR_GLITCH: name = "power glitch"; break;
            case ESP_RST_CPU_LOCKUP: name = "CPU lockup"; break;
            case ESP_RST_EFUSE:      name = "efuse error"; break;
            case ESP_RST_SDIO:       name = "SDIO"; abnormal = false; break;
            default: break;
        }
        if (abnormal) s_log.warn("reset reason: {}", name);
        else s_log.info("reset reason: {}", name);
    }
}

Engine::Engine()
    : m_inputs(m_tasks),
      m_fixtures(m_inputs, m_settings),
      m_render(m_fixtures, m_tasks),
      m_connectivity(m_wifi, m_tasks),
      m_statusReport(m_settings, m_wifi, m_fixtures, m_inputs),
      m_telemetry(m_settings.engine(), m_wifi, m_fixtures, m_statusReport, m_tasks),
      m_api(m_settings, m_fixtures, m_inputs, m_wifi, m_telemetry),
      m_describeHandler(m_api),
      m_getFixtureHandler(m_fixtures),
      m_setFixtureHandler(m_fixtures),
      m_inputsHandler(m_api),
      m_wifiConfigHandler(m_wifi, m_settings),
      m_rebootHandler(m_api),
      m_engineConfigHandler(m_api),
      m_serialSession(m_serial, m_dispatcher, m_tasks, "serial_cmd"),
      m_tcpListener(COMMAND_PORT, m_dispatcher, m_tasks)
{
    Utils::Log::begin();
    Utils::Log::setLevel(Utils::Level::Debug);
}

void Engine::init()
{
    logResetReason();

    s_log.debug("loading settings");
    m_settings.load();
    const Config::Engine& config = m_settings.engine();

    s_log.debug("connecting wifi");
    m_wifi.connect(config.ssid, config.password);

    s_log.debug("registering commands");
    registerCommands();

    // s_log.debug("starting BLE");
    // m_ble.begin("Lucke");

    s_log.debug("starting services");
    startServices();

    s_log.info("{} initialized", config.version);
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
    m_settings.start(m_tasks);
    m_inputs.start();

    // Animation until wifi first connects, then live DMX (see wireConnectivity)
    m_render.start();

    wireConnectivity();
    m_connectivity.start();

    m_telemetry.start();

    m_serialSession.start();
    m_tcpListener.start();
}
