#pragma once
#include "Interfaces/Command.h"
#include "API/API.h"


namespace Commands
{
    class InputsHandler : public Interface::CommandHandler
    {
    public:
        InputsHandler(API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        const API& m_api;
    };
}
