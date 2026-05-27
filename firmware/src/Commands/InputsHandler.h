#pragma once
#include "Interfaces/Command.h"
#include "Handlers/InputHandler.h"

namespace Commands
{
    class InputsHandler : public Interface::CommandHandler
    {
    public:
        InputsHandler(Handler::InputHandler& inputs);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        Handler::InputHandler& m_inputs;
    };
}
