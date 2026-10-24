#include "WifiConfigHandler.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"CMD"};
}

namespace Commands
{
    WifiConfigHandler::WifiConfigHandler(Net::Wifi& wifi, Services::SettingsStore& settings)
        : m_wifi(wifi), m_settings(settings)
    {}

    void WifiConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        bool hasSsid = req["ssid"].is<const char*>();
        bool hasPassword = req["password"].is<const char*>();
        bool dirty = hasSsid || hasPassword;

        if (dirty)
        {
            m_settings.update([&](Config::Engine& c) {
                if (hasSsid) c.ssid = req["ssid"].as<const char*>();
                if (hasPassword) c.password = req["password"].as<const char*>();
            });
        }
        Config::Engine config = m_settings.snapshot();

        if (dirty)
        {
            s_log.info("wifi settings changed via {}, ssid '{}'", transport.remoteAddress(), config.ssid);

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
