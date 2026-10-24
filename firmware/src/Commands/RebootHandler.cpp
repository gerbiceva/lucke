#include "RebootHandler.h"

namespace Commands
{
    RebootHandler::RebootHandler(Services::SettingsStore& settings)
        : m_settings(settings)
    {}

    void RebootHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        // Fixture records are written with a delay; don't lose the last change.
        m_settings.flush();
        transport.send("{\"status\":\"rebooting\"}");
        delay(1000);
        ESP.restart();
    }
}
