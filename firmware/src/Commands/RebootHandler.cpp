#include "RebootHandler.h"

namespace Commands
{
    RebootHandler::RebootHandler(API& api)
        : m_api(api)
    {}

    void RebootHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        // Restarts after a delay, so the reply still gets out.
        m_api.reboot(transport.remoteAddress());
        transport.send("{\"status\":\"rebooting\"}");
    }
}
