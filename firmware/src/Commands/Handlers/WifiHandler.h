#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    // "get_wifi" and "set_wifi".
    class WifiHandler : public Interface::CommandHandler
    {
    public:
        enum class Mode { Get, Set };

        WifiHandler(API& api, Mode mode);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        API& m_api;
        Mode m_mode;
    };
}
