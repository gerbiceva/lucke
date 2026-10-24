#pragma once
#include "Config/EngineConfig.h"
#include "Network/Wifi.h"
#include "Network/UdpBroadcaster.h"
#include "Network/SerialTransport.h"
#include "Network/BleTransport.h"
#include "Commands/CommandDispatcher.h"
#include "Commands/DescribeHandler.h"
#include "Commands/GetFixtureHandler.h"
#include "Commands/SetFixtureHandler.h"
#include "Commands/InputsHandler.h"
#include "Commands/WifiConfigHandler.h"
#include "Commands/RebootHandler.h"
#include "Commands/EngineConfigHandler.h"
#include "Handlers/FixtureHandler.h"
#include "Handlers/InputHandler.h"
#include "Tasks/TaskExecutor.h"
#include "Services/SettingsStore.h"
#include "Services/StatusReport.h"
#include "Services/Telemetry.h"
#include "Services/RenderLoop.h"
#include "Services/Connectivity.h"
#include "Services/CommandSession.h"
#include "Services/TcpListener.h"
#include "API/API.h"

class Engine
{
public:
    Engine();
    void init();

    template<typename TFixture>
    void addFixture()
    {
        m_fixtures.addFixture<TFixture>();
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

    Handler::InputHandler m_inputs;
    Handler::FixtureHandler m_fixtures;

    // Members are built in declaration order: each one below holds references
    // to members declared above it.
    Services::RenderLoop m_render;
    Services::Connectivity m_connectivity;
    Services::StatusReport m_statusReport;
    Services::Telemetry m_telemetry;
    API m_api;

    Commands::CommandDispatcher m_dispatcher;
    Commands::DescribeHandler m_describeHandler;
    Commands::GetFixtureHandler m_getFixtureHandler;
    Commands::SetFixtureHandler m_setFixtureHandler;
    Commands::InputsHandler m_inputsHandler;
    Commands::WifiConfigHandler m_wifiConfigHandler;
    Commands::RebootHandler m_rebootHandler;
    Commands::EngineConfigHandler m_engineConfigHandler;

    // Hold references to m_dispatcher, so they come after it.
    Services::CommandSession m_serialSession;
    Services::TcpListener m_tcpListener;
};
