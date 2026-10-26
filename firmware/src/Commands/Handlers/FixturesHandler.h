#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    // "fixtures": every fixture.
    class FixturesHandler : public Interface::CommandHandler
    {
    public:
        explicit FixturesHandler(const API& api);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        const API& m_api;
    };
}
