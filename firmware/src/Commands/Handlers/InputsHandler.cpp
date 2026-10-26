#include "InputsHandler.h"
#include "API/Wire.h"

namespace Commands
{
    InputsHandler::InputsHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply InputsHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        Interface::Success reply;
        Wire::inputs(reply.data.to<JsonArray>(), m_api.inputs());
        return reply;
    }
}
