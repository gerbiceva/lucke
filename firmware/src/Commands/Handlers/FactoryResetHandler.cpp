#include "FactoryResetHandler.h"
#include "Commands/ApiReply.h"

namespace Commands
{
    FactoryResetHandler::FactoryResetHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply FactoryResetHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        if (auto ok = requireConfirm(req); !ok) return std::unexpected(ok.error());

        // Restarts after a delay, so the dispatcher still gets the reply out.
        m_api.factoryReset(ctx.from);

        Interface::Success reply;
        reply.rebooting = true;
        return reply;
    }
}
