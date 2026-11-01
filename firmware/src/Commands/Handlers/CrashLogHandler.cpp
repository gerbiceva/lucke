#include "CrashLogHandler.h"
#include "API/Wire.h"

namespace Commands
{
    CrashLogHandler::CrashLogHandler(API& api, Mode mode)
        : m_api(api), m_mode(mode)
    {}

    Interface::Reply CrashLogHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        if (m_mode == Mode::Clear) m_api.clearCrashLog(ctx.from);

        Interface::Success reply;
        Wire::crashLog(reply.data.to<JsonObject>(), m_api.crashLog());
        return reply;
    }
}
