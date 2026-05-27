#include "InputsHandler.h"

namespace Commands
{
    InputsHandler::InputsHandler(Handler::InputHandler& inputs)
        : m_inputs(inputs)
    {}

    void InputsHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        m_inputs.toJson(obj);

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
