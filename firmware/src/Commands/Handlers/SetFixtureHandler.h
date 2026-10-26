#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    class SetFixtureHandler : public Interface::CommandHandler
    {
    public:
        explicit SetFixtureHandler(API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        API& m_api;
    };
}
