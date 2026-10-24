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

        m_tasks.spawnTask("report", [this]() {
            reportLoop();
        }, 1, 15000);
    }

    void Telemetry::setSerialReport(bool enable)
    {
        m_serialReport.store(enable);
    }

    void Telemetry::setWirelessReport(bool enable)
    {
        m_wirelessReport.store(enable);
    }

    void Telemetry::reportLoop()
    {
        while (true)
        {
            if (m_serialReport.load())
            {
                Utils::Logger::println(m_report.build().c_str());
            }
            if (m_wirelessReport.load() && m_wifi.isConnected())
            {
                m_udpReport.send(m_report.settings());
            }
            vTaskDelay(10000);
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
