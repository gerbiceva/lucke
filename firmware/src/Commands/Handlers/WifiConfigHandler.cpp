#include "WifiConfigHandler.h"

namespace Commands
{
    WifiConfigHandler::WifiConfigHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply WifiConfigHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        std::optional<std::string> ssid;
        std::optional<std::string> password;
        if (req["ssid"].is<const char*>()) ssid = req["ssid"].as<const char*>();
        if (req["password"].is<const char*>()) password = req["password"].as<const char*>();
        const bool dirty = ssid || password;

        // Reconnects after a delay, so the reply still gets out on the old link.
        if (dirty) m_api.setWifi(std::move(ssid), std::move(password), ctx.from);

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        if (dirty) obj["status"] = "ok";
        m_api.wifi().toJson(obj);
        obj["ssid"] = m_api.settings().ssid;
        return doc;
    }
}
