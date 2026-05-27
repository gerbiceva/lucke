#include "DescribeHandler.h"

namespace Commands
{
    DescribeHandler::DescribeHandler(Config::Engine& config, Handler::FixtureHandler& fixtures, Handler::InputHandler& inputs)
        : m_config(config), m_fixtures(fixtures), m_inputs(inputs)
    {}

    void DescribeHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();

        m_config.toJson(obj);

        JsonObject fixturesObj = obj["fixtures"].to<JsonObject>();
        m_fixtures.toJson(fixturesObj);

        JsonObject inputsObj = obj["inputs"].to<JsonObject>();
        m_inputs.toJson(inputsObj);

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
