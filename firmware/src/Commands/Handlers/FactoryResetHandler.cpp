#include "FactoryResetHandler.h"

namespace Commands
{
    FactoryResetHandler::FactoryResetHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply FactoryResetHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        // Restarts after a delay, so the dispatcher still gets the reply out.
        m_api.factoryReset(ctx.from);

        JsonDocument doc;
        doc["status"] = "factory reset, rebooting";
        return doc;
    }
}
