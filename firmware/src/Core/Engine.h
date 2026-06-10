#pragma once
#include "Config/EngineConfig.h"
#include "Network/Wifi.h"
#include "Network/SerialChannel.h"
#include "Network/TcpChannel.h"
#include "Network/BleChannel.h"
#include "Reports/Publisher.h"
#include "Reports/UdpSink.h"
#include "Reports/SerialSink.h"
#include "Reports/BleSink.h"
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
    void registerReports();
    void spawnTasks();
    void onWifiStatus(bool connected);
    std::string heartbeatJson();

    uint32_t m_fixtureTaskId = 0;
    uint32_t m_animationTaskId = 0;

    Config::Engine m_config;
    Utils::Storage m_storage;
    bool m_factoryReset = false;

    Network::Wifi m_wifi;

    Network::SerialChannel m_serialChannel;
    Network::TcpChannel m_tcpChannel;
    Network::BleChannel m_bleChannel;

    // Outbound periodic broadcasts (heartbeat / serial + wireless reports).
    Reports::Publisher m_publisher;
    Reports::UdpSink m_udpHeartbeatSink;
    Reports::UdpSink m_udpReportSink;
    Reports::SerialSink m_serialSink;
    Reports::BleSink m_bleSink;
    std::string m_heartbeatCache;

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
