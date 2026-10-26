#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    class SetFixtureHandler : public Interface::CommandHandler
    {
    public:
        explicit SetFixtureHandler(API& api);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        API& m_api;
    };
}
