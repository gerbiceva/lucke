#include "CommandDispatcher.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"CMD"};
}

namespace Commands
{
    void CommandDispatcher::registerHandler(const std::string& cmd, Interface::CommandHandler* handler)
    {
        m_handlers[cmd] = handler;
    }

    void CommandDispatcher::dispatch(const std::string& data, Interface::Transport& transport)
    {
        std::scoped_lock lock(m_mutex);
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data);
        if (err)
        {
            s_log.warn("JSON parse error: %s", err.c_str());
            transport.send("{\"err\":\"invalid json\"}");
            return;
        }

        const char* cmd = doc["request"] | "";
        if (cmd[0] == '\0')
        {
            s_log.warn("missing 'request' field");
            transport.send("{\"err\":\"missing cmd\"}");
            return;
        }

        auto it = m_handlers.find(cmd);
        if (it == m_handlers.end())
        {
            s_log.warn("unknown command: %s", cmd);
            transport.send("{\"err\":\"unknown cmd\"}");
            return;
        }

        it->second->handle(doc, transport);
    }
}
