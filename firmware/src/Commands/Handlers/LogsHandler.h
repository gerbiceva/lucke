#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    class LogsHandler : public Interface::CommandHandler
    {
    public:
        explicit LogsHandler(const API& api);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        const API& m_api;
    };
}
