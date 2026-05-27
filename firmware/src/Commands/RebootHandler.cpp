#include "RebootHandler.h"

namespace Commands
{
    void RebootHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        transport.send("{\"status\":\"rebooting\"}");
        delay(1000);
        ESP.restart();
    }
}
