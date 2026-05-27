#pragma once
#include "Config/EngineConfig.h"
#include "Network/Wifi.h"
#include "Network/TcpServer.h"
#include "Network/UdpBroadcaster.h"
#include "Network/SerialTransport.h"
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

    std::string toString();

private:
    void loadConfig();
    void saveConfig();
    void registerCommands();
    void spawnTasks();
    void onWifiStatus(bool connected);

    uint32_t m_fixtureTaskId = 0;
    uint32_t m_animationTaskId = 0;

    Config::Engine m_config;
    Utils::Storage m_storage;
    bool m_factoryReset = false;

    Network::Wifi m_wifi;
    Network::TcpServer m_tcpServer;
    Network::UdpBroadcaster m_udpHeartbeat;
    Network::UdpBroadcaster m_udpReport;
    Network::SerialTransport m_serial;

    Commands::CommandDispatcher m_dispatcher;
    Commands::DescribeHandler m_describeHandler;
    Commands::GetFixtureHandler m_getFixtureHandler;
    Commands::SetFixtureHandler m_setFixtureHandler;
    Commands::InputsHandler m_inputsHandler;
    Commands::WifiConfigHandler m_wifiConfigHandler;
    Commands::RebootHandler m_rebootHandler;
    Commands::EngineConfigHandler m_engineConfigHandler;

    Handler::InputHandler m_inputs;
    Handler::FixtureHandler m_fixtures;
    Tasks::Manager m_tasks;
};
