#include "InputsHandler.h"

namespace Commands
{
    InputsHandler::InputsHandler(API& api)
        : m_api(api)
    {}

    void InputsHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        m_api.inputs().toJson(obj);

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
