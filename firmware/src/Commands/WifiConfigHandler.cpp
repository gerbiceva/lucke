#include "WifiConfigHandler.h"

namespace Commands
{
    WifiConfigHandler::WifiConfigHandler(Net::Wifi& wifi, Services::SettingsStore& settings)
        : m_wifi(wifi), m_settings(settings)
    {}

    void WifiConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        Config::Engine& config = m_settings.engine();
        bool dirty = false;

        if (req["ssid"].is<const char*>())
        {
            config.ssid = req["ssid"].as<const char*>();
            dirty = true;
        }

        if (req["password"].is<const char*>())
        {
            config.password = req["password"].as<const char*>();
            dirty = true;
        }

        if (dirty)
        {
            m_settings.save();

            JsonDocument doc;
            JsonObject obj = doc.to<JsonObject>();
            obj["status"] = "ok";
            m_wifi.toJson(obj);
            obj["ssid"] = config.ssid;

            std::string response;
            serializeJson(doc, response);
            transport.send(response);

            delay(1000);
            m_wifi.reconnect(config.ssid, config.password);
        }
        else
        {
            JsonDocument doc;
            JsonObject obj = doc.to<JsonObject>();
            m_wifi.toJson(obj);
            obj["ssid"] = config.ssid;

            std::string response;
            serializeJson(doc, response);
            transport.send(response);
        }
    }
}
