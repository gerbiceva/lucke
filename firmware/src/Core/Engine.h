#pragma once
#include "Config/EngineConfig.h"
#include "Network/Wifi.h"
#include "Network/UdpBroadcaster.h"
#include "Network/SerialTransport.h"
#include "Network/BleTransport.h"
#include "Commands/CommandDispatcher.h"
#include "Commands/Handlers/DescribeHandler.h"
#include "Commands/Handlers/FixturesHandler.h"
#include "Commands/Handlers/GetFixtureHandler.h"
#include "Commands/Handlers/SetFixtureHandler.h"
#include "Commands/Handlers/InputsHandler.h"
#include "Commands/Handlers/WifiHandler.h"
#include "Commands/Handlers/RebootHandler.h"
#include "Commands/Handlers/SettingsHandler.h"
#include "Commands/Handlers/FactoryResetHandler.h"
#include "Commands/Handlers/LogsHandler.h"
#include "Commands/Handlers/OtaHandler.h"
#include "Core/Managers/FixtureManager.h"
#include "Core/Managers/InputManager.h"
#include "Tasks/TaskExecutor.h"
#include "Services/SettingsStore.h"
#include "Services/StatusReport.h"
#include "Services/Telemetry.h"
#include "Services/RenderLoop.h"
#include "Services/Connectivity.h"
#include "Services/CommandSession.h"
#include "Services/TcpListener.h"
#include "Services/OtaService.h"
#include "API/API.h"

class Engine
{
public:
    Engine();
    void init();

    // Strips take their default LED count: addFixture<StripWS2815<5>>(60).
    template<typename TFixture, typename... Args>
    void addFixture(Args... args)
    {
        m_fixtures.addFixture<TFixture>(args...);
    }

private:
    void registerCommands();
    void startServices();
    void wireConnectivity();

    // First: almost everything below reads settings.
    Services::SettingsStore m_settings;
    Tasks::Manager m_tasks;

    Net::Wifi m_wifi;
    Net::SerialTransport m_serial;
    // Net::BleTransport m_ble;

    Handler::InputManager m_inputs;
    Handler::FixtureManager m_fixtures;

    // Members are built in declaration order: each one below holds references
    // to members declared above it.
    Services::RenderLoop m_render;
    Services::Connectivity m_connectivity;
    Services::StatusReport m_statusReport;
    Services::Telemetry m_telemetry;
    Services::OtaService m_ota;
    API m_api;

    Commands::CommandDispatcher m_dispatcher;
    Commands::DescribeHandler m_describeHandler;
    Commands::FixturesHandler m_fixturesHandler;
    Commands::GetFixtureHandler m_getFixtureHandler;
    Commands::SetFixtureHandler m_setFixtureHandler;
    Commands::InputsHandler m_inputsHandler;
    Commands::WifiHandler m_getWifiHandler;
    Commands::WifiHandler m_setWifiHandler;
    Commands::RebootHandler m_rebootHandler;
    Commands::FactoryResetHandler m_factoryResetHandler;
    Commands::SettingsHandler m_getSettingsHandler;
    Commands::SettingsHandler m_setSettingsHandler;
    Commands::LogsHandler m_logsHandler;
    Commands::OtaHandler m_otaHandler;

    // Hold references to m_dispatcher, so they come after it.
    Services::CommandSession m_serialSession;
    Services::TcpListener m_tcpListener;
};
