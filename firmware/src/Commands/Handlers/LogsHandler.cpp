#include "LogsHandler.h"
#include "API/Wire.h"
#include "Commands/ApiReply.h"

namespace Commands
{
    LogsHandler::LogsHandler(const API& api)
        : m_api(api)
    {}

    Interface::Reply LogsHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        // {"cmd":"logs"} for everything kept, or "since": <next from the
        // previous reply> with its "boot_id" for only what's new.
        auto since = optInt(req, "since");
        auto bootId = optString(req, "boot_id");
        if (!since) return std::unexpected(since.error());
        if (!bootId) return std::unexpected(bootId.error());
        if (*since && **since < 0)
            return Interface::fail(Interface::Code::OutOfRange, "since must be 0 or more", "since");

        Interface::Success reply;
        Wire::logs(reply.data.to<JsonObject>(), m_api.logs(since->value_or(0), bootId->value_or("")));
        return reply;
    }
}
