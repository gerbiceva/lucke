#include "LogsHandler.h"

namespace Commands
{
    LogsHandler::LogsHandler(const API& api)
        : m_api(api)
    {}

    Interface::Reply LogsHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        // {"request":"logs"} for everything kept, or "since": <next from
        // the previous reply> for only what's new.
        const uint32_t since = req["since"] | 0u;

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        m_api.logs(since).toJson(obj);
        return doc;
    }
}
