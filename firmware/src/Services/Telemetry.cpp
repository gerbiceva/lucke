#include "Telemetry.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"

namespace Services
{
    Telemetry::Telemetry(const Config::Engine& config, const Net::Wifi& wifi, const Handler::FixtureHandler& fixtures,
                         StatusReport& report, Tasks::Manager& tasks)
        : m_config(config), m_wifi(wifi), m_fixtures(fixtures), m_report(report), m_tasks(tasks)
    {}

    void Telemetry::start()
    {
        m_udpHeartbeat.begin(HEARTBEAT_PORT);
        m_udpReport.begin(REPORT_PORT);

        m_heartbeatTaskId = m_tasks.spawnTask("heartbeat", [this]() {
            heartbeatLoop();
        }, 1, 4096);

        setSerialReport(m_config.serial_report_task);
        setWirelessReport(m_config.wireless_report_task);
    }

    void Telemetry::setSerialReport(bool enable)
    {
        bool running = m_serialReportTaskId != Tasks::TaskId::Invalid;
        if (enable && !running)
        {
            m_serialReportTaskId = m_tasks.spawnTask("serial_report", [this]() {
                while (true)
                {
                    Utils::Logger::println(m_report.build().c_str());
                    vTaskDelay(10000);
                }
            }, 1, 15000);
        }
        else if (!enable && running)
        {
            m_tasks.stopTask(m_serialReportTaskId);
            m_serialReportTaskId = Tasks::TaskId::Invalid;
        }
    }

    void Telemetry::setWirelessReport(bool enable)
    {
        bool running = m_wirelessReportTaskId != Tasks::TaskId::Invalid;
        if (enable && !running)
        {
            m_wirelessReportTaskId = m_tasks.spawnTask("wireless_report", [this]() {
                while (true)
                {
                    if (m_wifi.isConnected())
                    {
                        m_udpReport.send(m_report.settings());
                    }
                    vTaskDelay(10000);
                }
            }, 1, 8192);
        }
        else if (!enable && running)
        {
            m_tasks.stopTask(m_wirelessReportTaskId);
            m_wirelessReportTaskId = Tasks::TaskId::Invalid;
        }
    }

    void Telemetry::heartbeatLoop()
    {
        while (true)
        {
            // Rebuilt every beat so renamed fixtures show up without a reboot.
            if (m_wifi.isConnected())
            {
                m_udpHeartbeat.send(buildHeartbeat());
            }
            vTaskDelay(500);
        }
    }

    std::string Telemetry::buildHeartbeat() const
    {
        JsonDocument doc;
        doc["version"] = ENGINE_VERSION;
        JsonArray arr = doc["fixtures"].to<JsonArray>();
        for (const auto& fix : m_fixtures.all())
        {
            JsonObject entry = arr.add<JsonObject>();
            entry["name"] = fix->getName();
            entry["type"] = fix->getType();
        }

        std::string out;
        serializeJson(doc, out);
        return out;
    }
}
