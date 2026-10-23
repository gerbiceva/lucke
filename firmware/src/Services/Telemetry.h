#pragma once
#include <string>
#include "Core/Config/EngineConfig.h"
#include "Network/Wifi.h"
#include "Network/UdpBroadcaster.h"
#include "Handlers/FixtureHandler.h"
#include "Services/StatusReport.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // Periodic outbound status: the UDP heartbeat that lets clients discover
    // the device, plus the optional serial and UDP status reports. Reads
    // state, never mutates or persists it — the caller owns the settings.
    class Telemetry
    {
    public:
        Telemetry(const Config::Engine& config, const Net::Wifi& wifi, const Handler::FixtureHandler& fixtures,
                  StatusReport& report, Tasks::Manager& tasks);

        // Opens the UDP sockets, spawns the heartbeat and starts the reports
        // enabled in config. Call once, after everything this depends on is
        // constructed.
        void start();

        // Start/stop the report tasks. Idempotent.
        void setSerialReport(bool enable);
        void setWirelessReport(bool enable);

    private:
        void heartbeatLoop();
        std::string buildHeartbeat() const;

        const Config::Engine& m_config;
        const Net::Wifi& m_wifi;
        const Handler::FixtureHandler& m_fixtures;
        StatusReport& m_report;
        Tasks::Manager& m_tasks;

        Net::UdpBroadcaster m_udpHeartbeat;
        Net::UdpBroadcaster m_udpReport;

        Tasks::TaskId m_heartbeatTaskId = Tasks::TaskId::Invalid;
        Tasks::TaskId m_serialReportTaskId = Tasks::TaskId::Invalid;
        Tasks::TaskId m_wirelessReportTaskId = Tasks::TaskId::Invalid;
    };
}
