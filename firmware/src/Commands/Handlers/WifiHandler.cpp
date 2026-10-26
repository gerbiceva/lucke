#include "WifiHandler.h"
#include "API/Wire.h"
#include "Commands/ApiReply.h"

namespace Commands
{
    WifiHandler::WifiHandler(API& api, Mode mode)
        : m_api(api), m_mode(mode)
    {}

    Interface::Reply WifiHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        if (m_mode == Mode::Set)
        {
            auto ssid = optString(req, "ssid");
            auto password = optString(req, "password");
            if (!ssid) return std::unexpected(ssid.error());
            if (!password) return std::unexpected(password.error());
            if (!*ssid) return missing("ssid");
            if (auto ok = requireConfirm(req); !ok) return std::unexpected(ok.error());

            // Reconnects after a delay, so the reply still gets out on the old link.
            m_api.setWifi(std::move(*ssid), std::move(*password), ctx.from);
        }

        Interface::Success reply;
        Wire::wifi(reply.data.to<JsonObject>(), m_api.wifi(), m_api.settings().ssid);
        return reply;
    }
}
