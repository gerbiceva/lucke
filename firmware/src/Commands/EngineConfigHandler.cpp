#include "EngineConfigHandler.h"
#include "Utils/Logger.h"

namespace Commands
{
    EngineConfigHandler::EngineConfigHandler(Services::SettingsStore& settings, Services::Telemetry& telemetry)
        : m_settings(settings), m_telemetry(telemetry)
    {}

    void EngineConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
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
                m_settings.update([value](Config::Engine& c) { c.serial_report_task = value; });
            }
            respond(transport, "serial_report_task", m_settings.snapshot().serial_report_task);
            return;
        }

        if (strcmp(request, "wireless_report_task") == 0)
        {
            if (req["value"].is<bool>())
            {
                bool value = req["value"];
                m_telemetry.setWirelessReport(value);
                m_settings.update([value](Config::Engine& c) { c.wireless_report_task = value; });
            }
            respond(transport, "wireless_report_task", m_settings.snapshot().wireless_report_task);
            return;
        }

        if (strcmp(request, "wifi_animation") == 0)
        {
            if (req["value"].is<bool>())
            {
                bool value = req["value"];
                m_settings.update([value](Config::Engine& c) { c.wifi_animation = value; });
            }
            respond(transport, "wifi_animation", m_settings.snapshot().wifi_animation);
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
