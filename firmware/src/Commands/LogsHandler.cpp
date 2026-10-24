#include "LogsHandler.h"

namespace Commands
{
    LogsHandler::LogsHandler(const API& api)
        : m_api(api)
    {}

    void LogsHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        // {"request":"logs"} for everything kept, or "since": <next from
        // the previous reply> for only what's new.
        const uint32_t since = req["since"] | 0u;

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        m_api.logs(since).toJson(obj);

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
