#pragma once
#include <Arduino.h>
#include "Interfaces/Command.h"

namespace Commands
{
    class RebootHandler : public Interface::CommandHandler
    {
    public:
        void handle(JsonDocument& req, Interface::Transport& transport) override;
    };
}
