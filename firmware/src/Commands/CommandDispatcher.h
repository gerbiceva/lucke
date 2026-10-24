#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <ArduinoJson.h>
#include "Interfaces/Command.h"
#include "Interfaces/Transport.h"

namespace Commands
{
    class CommandDispatcher
    {
    public:
        void registerHandler(const std::string& cmd, Interface::CommandHandler* handler);
        void dispatch(const std::string& data, Interface::Transport& transport);

    private:
        std::unordered_map<std::string, Interface::CommandHandler*> m_handlers;
        // Serial and TCP commands arrive on different tasks. Running one at a
        // time means handlers never race each other on fixtures or settings.
        std::mutex m_mutex;
    };
}
