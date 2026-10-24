#include "RebootHandler.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"CMD"};
}

namespace Commands
{
    RebootHandler::RebootHandler(Services::SettingsStore& settings)
        : m_settings(settings)
    {}

    void RebootHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        s_log.info("reboot requested via {}", transport.remoteAddress());
        // Fixture records are written with a delay; don't lose the last change.
        m_settings.flush();
        transport.send("{\"status\":\"rebooting\"}");
        delay(1000);
        ESP.restart();
    }
}
