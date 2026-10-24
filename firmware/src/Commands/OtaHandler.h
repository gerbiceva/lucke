#pragma once
#include "Interfaces/Command.h"
#include "API/API.h"

namespace Commands
{
    class OtaHandler : public Interface::CommandHandler
    {
    public:
        explicit OtaHandler(API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        API& m_api;
    };
}
