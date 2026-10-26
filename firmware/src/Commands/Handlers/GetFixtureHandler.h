#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    // "get_fixture": one fixture, optionally with its outputs and presets.
    class GetFixtureHandler : public Interface::CommandHandler
    {
    public:
        explicit GetFixtureHandler(const API& api);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        const API& m_api;
    };
}
