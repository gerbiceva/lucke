#include "SettingsHandler.h"
#include "API/Wire.h"
#include "Commands/ApiReply.h"

namespace Commands
{
    SettingsHandler::SettingsHandler(API& api, Mode mode)
        : m_api(api), m_mode(mode)
    {}

    Interface::Reply SettingsHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        if (m_mode == Mode::Set)
        {
            // Any subset; check all before changing any.
            auto serial = optBool(req, "serial_report");
            auto wireless = optBool(req, "wireless_report");
            auto animation = optBool(req, "wifi_animation");
            if (!serial) return std::unexpected(serial.error());
            if (!wireless) return std::unexpected(wireless.error());
            if (!animation) return std::unexpected(animation.error());

            if (*serial) m_api.setSerialReport(**serial, ctx.from);
            if (*wireless) m_api.setWirelessReport(**wireless, ctx.from);
            if (*animation) m_api.setWifiAnimation(**animation, ctx.from);
        }

        Interface::Success reply;
        Wire::settings(reply.data.to<JsonObject>(), m_api.settings());
        return reply;
    }
}
