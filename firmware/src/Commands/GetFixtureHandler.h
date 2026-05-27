#pragma once
#include "Interfaces/Command.h"
#include "Handlers/FixtureHandler.h"

namespace Commands
{
    class GetFixtureHandler : public Interface::CommandHandler
    {
    public:
        GetFixtureHandler(Handler::FixtureHandler& fixtures);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Handler::FixtureHandler& m_fixtures;
    };
}
