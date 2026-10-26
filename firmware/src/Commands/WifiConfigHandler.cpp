#include "WifiConfigHandler.h"

namespace Commands
{
    WifiConfigHandler::WifiConfigHandler(API& api)
        : m_api(api)
    {}

    void WifiConfigHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        std::optional<std::string> ssid;
        std::optional<std::string> password;
        if (req["ssid"].is<const char*>()) ssid = req["ssid"].as<const char*>();
        if (req["password"].is<const char*>()) password = req["password"].as<const char*>();
        const bool dirty = ssid || password;

        // Reconnects after a delay, so the reply still gets out on the old link.
        if (dirty) m_api.setWifi(std::move(ssid), std::move(password), transport.remoteAddress());

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        if (dirty) obj["status"] = "ok";
        m_api.wifi().toJson(obj);
        obj["ssid"] = m_api.settings().ssid;
        transport.send(doc);
    }
}
