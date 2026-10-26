#include "DescribeHandler.h"
#include "API/Wire.h"

namespace Commands
{
    DescribeHandler::DescribeHandler(const API& api)
        : m_api(api)
    {}

    Interface::Reply DescribeHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        Interface::Success reply;
        Wire::describe(reply.data.to<JsonObject>(), m_api.describe());
        return reply;
    }
}
