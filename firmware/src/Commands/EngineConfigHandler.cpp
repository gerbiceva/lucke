#include "EngineConfigHandler.h"
#include "Utils/Logger.h"

namespace Commands
{
    EngineConfigHandler::EngineConfigHandler(Services::SettingsStore& settings, Services::Telemetry& telemetry)
        : m_settings(settings), m_telemetry(telemetry)
    {}

    void EngineConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        Config::Engine& config = m_settings.engine();
        const char* request = req["request"] | "";

        if (strcmp(request, "factory_reset") == 0)
        {
            m_settings.factoryReset();
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
                m_telemetry.setSerialReport(value);
                config.serial_report_task = value;
                m_settings.save();
            }
            respond(transport, "serial_report_task", config.serial_report_task);
            return;
        }

        if (strcmp(request, "wireless_report_task") == 0)
        {
            if (req["value"].is<bool>())
            {
                bool value = req["value"];
                m_telemetry.setWirelessReport(value);
                config.wireless_report_task = value;
                m_settings.save();
            }
            respond(transport, "wireless_report_task", config.wireless_report_task);
            return;
        }

        if (strcmp(request, "wifi_animation") == 0)
        {
            if (req["value"].is<bool>())
            {
                config.wifi_animation = req["value"];
                m_settings.save();
            }
            respond(transport, "wifi_animation", config.wifi_animation);
            return;
        }

        transport.send("{\"err\":\"unknown setting\"}");
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
}
