#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    class RebootHandler : public Interface::CommandHandler
    {
    public:
        explicit RebootHandler(API& api);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        API& m_api;
    };
}
