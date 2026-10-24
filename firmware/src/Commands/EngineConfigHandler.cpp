#include "EngineConfigHandler.h"
#include <cstring>

namespace Commands
{
    EngineConfigHandler::EngineConfigHandler(API& api)
        : m_api(api)
    {}

    void EngineConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        const char* request = req["request"] | "";
        const std::string from = transport.remoteAddress();
        const bool hasValue = req["value"].is<bool>();
        const bool value = req["value"] | false;

        if (strcmp(request, "factory_reset") == 0)
        {
            // Restarts after a delay, so the reply still gets out.
            m_api.factoryReset(from);
            transport.send("{\"status\":\"factory reset, rebooting\"}");
            return;
        }

        if (strcmp(request, "serial_report_task") == 0)
        {
            if (hasValue) m_api.setSerialReport(value, from);
            respond(transport, "serial_report_task", m_api.settings().serial_report_task);
            return;
        }

        if (strcmp(request, "wireless_report_task") == 0)
        {
            if (hasValue) m_api.setWirelessReport(value, from);
            respond(transport, "wireless_report_task", m_api.settings().wireless_report_task);
            return;
        }

        if (strcmp(request, "wifi_animation") == 0)
        {
            if (hasValue) m_api.setWifiAnimation(value, from);
            respond(transport, "wifi_animation", m_api.settings().wifi_animation);
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
