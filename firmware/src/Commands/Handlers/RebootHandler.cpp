#include "RebootHandler.h"

namespace Commands
{
    RebootHandler::RebootHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply RebootHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        // Restarts after a delay, so the dispatcher still gets the reply out.
        m_api.reboot(ctx.from);

        JsonDocument doc;
        doc["status"] = "rebooting";
        return doc;
    }
}
