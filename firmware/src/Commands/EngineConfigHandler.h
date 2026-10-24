#pragma once
#include "Interfaces/Command.h"
#include "API/API.h"

namespace Commands
{
    class EngineConfigHandler : public Interface::CommandHandler
    {
    public:
        explicit EngineConfigHandler(API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        API& m_api;

        void respond(Interface::Transport& transport, const char* key, bool value);
    };
}
