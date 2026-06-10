#include "EngineConfigHandler.h"
#include "Utils/Logger.h"

namespace Commands
{
    EngineConfigHandler::EngineConfigHandler(Config::Engine& config, Utils::Storage& storage, Reports::Publisher& publisher)
        : m_config(config), m_storage(storage), m_publisher(publisher)
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
                m_config.serial_report_task = req["value"];
                m_publisher.setEnabled(SERIAL_REPORT_FEED, m_config.serial_report_task);
                save();
            }
            respond(transport, "serial_report_task", m_config.serial_report_task);
            return;
        }

        if (strcmp(request, "wireless_report_task") == 0)
        {
            if (req["value"].is<bool>())
            {
                m_config.wireless_report_task = req["value"];
                m_publisher.setEnabled(WIRELESS_REPORT_FEED, m_config.wireless_report_task);
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
}
