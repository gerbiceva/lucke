#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    class RebootHandler : public Interface::CommandHandler
    {
    public:
        explicit RebootHandler(API& api);
        void handle(JsonDocument& req, Interface::Transport& transport) override;

    private:
        API& m_api;
    };
}
