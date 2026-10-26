#pragma once
#include <functional>
#include <mutex>
#include <unordered_map>
#include <string>
#include <string_view>
#include <ArduinoJson.h>
#include "Commands/Command.h"
#include "Interfaces/Transport.h"

namespace Commands
{
    class CommandDispatcher
    {
    public:
        void registerHandler(const std::string& cmd, Interface::CommandHandler* handler);
        void dispatch(const std::string& data, Interface::Transport& transport);

    private:
        // Lets find() take a string_view, so a lookup doesn't build a std::string.
        struct StringHash
        {
            using is_transparent = void;
            size_t operator()(std::string_view s) const { return std::hash<std::string_view>{}(s); }
        };

        Interface::Reply run(const std::string& data, const Interface::Context& ctx);

        std::unordered_map<std::string, Interface::CommandHandler*, StringHash, std::equal_to<>> m_handlers;
        // Serial and TCP commands arrive on different tasks. Running one at a
        // time means handlers never race each other on fixtures or settings.
        std::mutex m_mutex;
    };
}
