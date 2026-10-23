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
#include "Utils/Storage.h"
#include "Services/StatusReport.h"
#include "Services/Telemetry.h"
#include "Services/CommandSession.h"
#include "Services/TcpListener.h"

class Engine
{
public:
    Engine();
    void init();

    template<typename TFixture>
    void addFixture()
    {
        m_fixtures.addFixture<TFixture>(m_factoryReset);
    }

private:
    void loadConfig();
    void saveConfig();
    void registerCommands();
    void spawnTasks();
    void onWifiStatus(bool connected);

    Tasks::TaskId m_fixtureTaskId = Tasks::TaskId::Invalid;
    Tasks::TaskId m_animationTaskId = Tasks::TaskId::Invalid;

    Config::Engine m_config;
    Utils::Storage m_storage;
    bool m_factoryReset = false;

    Net::Wifi m_wifi;
    Net::SerialTransport m_serial;
    // Net::BleTransport m_ble;

    Handler::InputHandler m_inputs;
    Handler::FixtureHandler m_fixtures;
    Tasks::Manager m_tasks;

    // Members are built in declaration order: each one below holds references
    // to members declared above it.
    Services::StatusReport m_statusReport;
    Services::Telemetry m_telemetry;

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
