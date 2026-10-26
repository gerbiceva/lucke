#include "DescribeHandler.h"

namespace Commands
{
    DescribeHandler::DescribeHandler(const API& api)
        : m_api(api)
    {}

    Interface::Reply DescribeHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        m_api.describe().toJson(obj);
        return doc;
    }
}
