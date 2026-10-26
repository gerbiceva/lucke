#include "InputsHandler.h"

namespace Commands
{
    InputsHandler::InputsHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply InputsHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        m_api.inputs().toJson(obj);
        return doc;
    }
}
