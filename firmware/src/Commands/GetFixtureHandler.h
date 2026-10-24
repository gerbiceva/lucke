#pragma once
#include "Interfaces/Command.h"
#include "API/API.h"

namespace Commands
{
    class GetFixtureHandler : public Interface::CommandHandler
    {
    public:
        explicit GetFixtureHandler(const API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        const API& m_api;
    };
}
