#pragma once
#include "Commands/Command.h"
#include "API/API.h"


namespace Commands
{
    class InputsHandler : public Interface::CommandHandler
    {
    public:
        InputsHandler(API& api);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        const API& m_api;
    };
}
