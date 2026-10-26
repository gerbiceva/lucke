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
        Interface::Reply reply = run(data, Interface::Context{transport.remoteAddress()});

        JsonDocument out;
        if (reply)
        {
            out["ok"] = true;
            if (reply->rebooting) out["rebooting"] = true;
            if (reply->data.isNull()) out["data"].to<JsonObject>();
            else out["data"].set(reply->data);
        }
        else
        {
            const Interface::Error& e = reply.error();
            out["ok"] = false;
            JsonObject err = out["err"].to<JsonObject>();
            err["code"] = e.code;
            if (!e.arg.empty()) err["arg"] = e.arg;
            err["msg"] = e.msg;
        }
        transport.send(out);
    }

    Interface::Reply CommandDispatcher::run(const std::string& data, const Interface::Context& ctx)
    {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data);
        if (err)
        {
            s_log.warn("JSON parse error: %s", err.c_str());
            return Interface::fail(Interface::Code::InvalidJson, "invalid json");
        }

        const char* cmd = doc["cmd"] | "";
        if (cmd[0] == '\0')
        {
            s_log.warn("missing 'cmd' field");
            return Interface::fail(Interface::Code::MissingCmd, "missing cmd", "cmd");
        }

        auto it = m_handlers.find(std::string_view{cmd});
        if (it == m_handlers.end())
        {
            s_log.warn("unknown command: %s", cmd);
            return Interface::fail(Interface::Code::UnknownCmd, "unknown cmd");
        }

        return it->second->handle(doc, ctx);
    }
}
