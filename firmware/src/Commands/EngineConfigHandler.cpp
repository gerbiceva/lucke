#include "EngineConfigHandler.h"
#include "Utils/Logger.h"

namespace Commands
{
    EngineConfigHandler::EngineConfigHandler(Config::Engine& config, Utils::Storage& storage, Tasks::Manager& tasks)
        : m_config(config), m_storage(storage), m_tasks(tasks)
    {}

    void EngineConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        const char* request = req["request"] | "";

        if (strcmp(request, "factory_reset") == 0)
        {
            m_config.to_factory_settings = true;
            save();
            transport.send("{\"status\":\"factory reset, rebooting\"}");
            delay(1000);
            ESP.restart();
            return;
        }

        if (strcmp(request, "serial_report_task") == 0)
        {
            if (req["value"].is<bool>())
            {
                bool value = req["value"];
                toggleSerialReport(value);
                m_config.serial_report_task = value;
                save();
            }
            respond(transport, "serial_report_task", m_config.serial_report_task);
            return;
        }

        if (strcmp(request, "wireless_report_task") == 0)
        {
            if (req["value"].is<bool>())
            {
                bool value = req["value"];
                toggleWirelessReport(value);
                m_config.wireless_report_task = value;
                save();
            }
            respond(transport, "wireless_report_task", m_config.wireless_report_task);
            return;
        }

        if (strcmp(request, "wifi_animation") == 0)
        {
            if (req["value"].is<bool>())
            {
                m_config.wifi_animation = req["value"];
                save();
            }
            respond(transport, "wifi_animation", m_config.wifi_animation);
            return;
        }

        transport.send("{\"err\":\"unknown setting\"}");
    }

    void EngineConfigHandler::save()
    {
        m_storage.putString("settings", m_config.toString());
    }

    void EngineConfigHandler::respond(Interface::Transport& transport, const char* key, bool value)
    {
        JsonDocument doc;
        doc["req"] = key;
        doc["val"] = value;

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }

    void EngineConfigHandler::toggleSerialReport(bool enable)
    {
        if (enable && !m_config.serial_report_task)
        {
            m_serialReportTaskId = m_tasks.spawnTask("serial_report", [this]() {
                while (true)
                {
                    Utils::Logger::println(m_config.toString().c_str());
                    vTaskDelay(10000);
                }
            }, 1, 4096);
        }
        else if (!enable && m_config.serial_report_task)
        {
            m_tasks.stopTask(m_serialReportTaskId);
        }
    }

    void EngineConfigHandler::toggleWirelessReport(bool enable)
    {
        if (enable && !m_config.wireless_report_task)
        {
            m_wirelessReportTaskId = m_tasks.spawnTask("wireless_report", [this]() {
                while (true)
                {
                    vTaskDelay(10000);
                }
            }, 1, 4096);
        }
        else if (!enable && m_config.wireless_report_task)
        {
            m_tasks.stopTask(m_wirelessReportTaskId);
        }
    }
}
