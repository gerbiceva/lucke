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
        const Interface::Reply reply = run(data, Interface::Context{transport.remoteAddress()});
        if (reply)
        {
            transport.send(*reply);
            return;
        }

        JsonDocument err;
        err["err"] = reply.error();
        transport.send(err);
    }

    Interface::Reply CommandDispatcher::run(const std::string& data, const Interface::Context& ctx)
    {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data);
        if (err)
        {
            s_log.warn("JSON parse error: %s", err.c_str());
            return std::unexpected("invalid json");
        }

        const char* cmd = doc["request"] | "";
        if (cmd[0] == '\0')
        {
            s_log.warn("missing 'request' field");
            return std::unexpected("missing cmd");
        }

        auto it = m_handlers.find(cmd);
        if (it == m_handlers.end())
        {
            s_log.warn("unknown command: %s", cmd);
            return std::unexpected("unknown cmd");
        }

        return it->second->handle(doc, ctx);
    }
}
