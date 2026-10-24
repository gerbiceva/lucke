#pragma once
#include <atomic>
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

        // Opens the UDP sockets and spawns the heartbeat and report tasks,
        // with the reports enabled in config. Call once, after everything
        // this depends on is constructed.
        void start();

        // Enable/disable a report. Takes effect on the next report cycle.
        // Safe to call from any task.
        void setSerialReport(bool enable);
        void setWirelessReport(bool enable);

    private:
        void heartbeatLoop();
        void reportLoop();
        std::string buildHeartbeat() const;

        const Config::Engine& m_config;
        const Net::Wifi& m_wifi;
        const Handler::FixtureHandler& m_fixtures;
        StatusReport& m_report;
        Tasks::Manager& m_tasks;

        Net::UdpBroadcaster m_udpHeartbeat;
        Net::UdpBroadcaster m_udpReport;

        // The report task runs forever and checks these: deleting it from
        // outside could kill it while it holds a lock (InputHandler's mutex
        // during build()), freezing whoever needs that lock next.
        std::atomic<bool> m_serialReport{false};
        std::atomic<bool> m_wirelessReport{false};
    };
}
