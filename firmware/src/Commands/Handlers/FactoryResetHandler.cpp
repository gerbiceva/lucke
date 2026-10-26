#include "FactoryResetHandler.h"

namespace Commands
{
    FactoryResetHandler::FactoryResetHandler(API& api)
        : m_api(api)
    {}

    void FactoryResetHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        // Restarts after a delay, so the reply still gets out.
        m_api.factoryReset(transport.remoteAddress());
        transport.send("{\"status\":\"factory reset, rebooting\"}");
    }
}
