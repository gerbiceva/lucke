#pragma once
#include "Commands/Command.h"
#include "API/API.h"

namespace Commands
{
    // "crash_log" and "clear_crash_log".
    class CrashLogHandler : public Interface::CommandHandler
    {
    public:
        enum class Mode { Get, Clear };

        CrashLogHandler(API& api, Mode mode);
        Interface::Reply handle(const JsonDocument& req, const Interface::Context& ctx) override;

    private:
        API& m_api;
        Mode m_mode;
    };
}
