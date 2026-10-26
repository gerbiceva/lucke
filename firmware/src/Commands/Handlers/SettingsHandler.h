#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    // "get_settings" and "set_settings": the report switches and wifi animation.
    class SettingsHandler : public Interface::CommandHandler
    {
    public:
        enum class Mode { Get, Set };

        SettingsHandler(API& api, Mode mode);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        API& m_api;
        Mode m_mode;
    };
}
