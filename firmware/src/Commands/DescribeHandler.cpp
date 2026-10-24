#include "DescribeHandler.h"

namespace Commands
{
    DescribeHandler::DescribeHandler(const API& api)
        : m_api(api)
    {}

    void DescribeHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        m_api.describe().toJson(obj);

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
