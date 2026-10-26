#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    class LogsHandler : public Interface::CommandHandler
    {
    public:
        explicit LogsHandler(const API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        const API& m_api;
    };
}
