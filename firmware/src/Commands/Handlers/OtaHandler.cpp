#include "OtaHandler.h"
#include "Commands/ApiReply.h"

namespace Commands
{
    OtaHandler::OtaHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply OtaHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        // {"cmd":"ota"} or {"cmd":"ota","minutes":10}; 1..30, default 5.
        auto requested = optInt(req, "minutes");
        if (!requested) return std::unexpected(requested.error());
        const int32_t minutes = requested->value_or(5);
        if (minutes < 1 || minutes > 30)
            return Interface::fail(Interface::Code::OutOfRange, "minutes must be 1-30", "minutes");

        m_api.enableOta(minutes, ctx.from);

        Interface::Success reply;
        JsonObject data = reply.data.to<JsonObject>();
        data["minutes"] = minutes;
        data["port"] = OTA_PORT;
        return reply;
    }
}
