#include "CommandDispatcher.h"
#include "Utils/Logger.h"

namespace Commands
{
    void CommandDispatcher::registerHandler(const std::string& cmd, Interface::CommandHandler* handler)
    {
        m_handlers[cmd] = handler;
    }

    void CommandDispatcher::dispatch(const std::string& data, Interface::Transport& transport)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data);
        if (err)
        {
            Utils::Logger::printf("[CMD] JSON parse error: %s\n", err.c_str());
            transport.send("{\"err\":\"invalid json\"}");
            return;
        }

        const char* cmd = doc["request"] | "";
        if (cmd[0] == '\0')
        {
            Utils::Logger::println("[CMD] Missing 'cmd' field");
            transport.send("{\"err\":\"missing cmd\"}");
            return;
        }

        auto it = m_handlers.find(cmd);
        if (it == m_handlers.end())
        {
            Utils::Logger::printf("[CMD] Unknown command: %s\n", cmd);
            transport.send("{\"err\":\"unknown cmd\"}");
            return;
        }

        it->second->handle(doc, transport);
    }
}
