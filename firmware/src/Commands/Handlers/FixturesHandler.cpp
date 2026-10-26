#include "FixturesHandler.h"
#include "API/Wire.h"

namespace Commands
{
    FixturesHandler::FixturesHandler(const API& api)
        : m_api(api)
    {}

    Interface::Reply FixturesHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        Interface::Success reply;
        Wire::fixtures(reply.data.to<JsonArray>(), m_api.fixtures());
        return reply;
    }
}
