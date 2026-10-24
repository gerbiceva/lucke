#pragma once
#include "Interfaces/Command.h"
#include "API/API.h"

namespace Commands
{
    class WifiConfigHandler : public Interface::CommandHandler
    {
    public:
        explicit WifiConfigHandler(API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        API& m_api;
    };
}
