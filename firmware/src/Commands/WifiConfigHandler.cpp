#include "WifiConfigHandler.h"

namespace Commands
{
    WifiConfigHandler::WifiConfigHandler(Network::Wifi& wifi, Config::Engine& config, Utils::Storage& storage)
        : m_wifi(wifi), m_config(config), m_storage(storage)
    {}

    void WifiConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        bool dirty = false;

        if (req["ssid"].is<const char*>())
        {
            m_config.ssid = req["ssid"].as<const char*>();
            dirty = true;
        }

        if (req["password"].is<const char*>())
        {
            m_config.password = req["password"].as<const char*>();
            dirty = true;
        }

        if (dirty)
        {
            m_storage.putString("settings", m_config.toString());

            JsonDocument doc;
            JsonObject obj = doc.to<JsonObject>();
            obj["status"] = "ok";
            m_wifi.toJson(obj);
            obj["ssid"] = m_config.ssid;

            std::string response;
            serializeJson(doc, response);
            transport.send(response);

            delay(1000);
            m_wifi.reconnect(m_config.ssid, m_config.password);
        }
        else
        {
            JsonDocument doc;
            JsonObject obj = doc.to<JsonObject>();
            m_wifi.toJson(obj);
            obj["ssid"] = m_config.ssid;

            std::string response;
            serializeJson(doc, response);
            transport.send(response);
        }
    }
}
