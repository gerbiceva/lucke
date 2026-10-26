#include "GetFixtureHandler.h"

namespace
{
    void sendError(Interface::Transport& transport, const char* err)
    {
        JsonDocument doc;
        doc["err"] = err;
        transport.send(doc);
    }
}

namespace Commands
{
    GetFixtureHandler::GetFixtureHandler(const API& api)
        : m_api(api)
    {}

    void GetFixtureHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();

        // No id: every fixture.
        if (!req["id"].is<uint8_t>())
        {
            m_api.fixtures().toJson(obj);
            transport.send(doc);
            return;
        }

        const uint8_t id = req["id"];
        auto fixture = m_api.fixture(id);
        if (!fixture)
        {
            sendError(transport, toString(fixture.error()));
            return;
        }

        // No value: the whole fixture.
        if (!req["value"].is<const char*>())
        {
            fixture->toJson(obj);
            transport.send(doc);
            return;
        }

        // One field: {"req": field, "val": ...}
        const std::string field = req["value"].as<const char*>();
        const Config::Fixture& config = fixture->config;
        doc.clear();
        doc["req"] = field;
        if (field == "universe")      doc["val"] = config.universe;
        else if (field == "address")  doc["val"] = config.address;
        else if (field == "preset")   doc["val"] = config.selectedPreset;
        else if (field == "name")     doc["val"] = config.name;
        else if (field == "type")     doc["val"] = config.type;
        else if (field == "leds")     doc["val"] = config.numLeds;
        else if (field == "config")   doc["val"] = *m_api.fixtureConfig(id);
        else if (field == "presets")  doc["val"] = *m_api.fixturePresets(id);
        else if (field == "outputs")  doc["val"] = *m_api.fixtureOutputs(id);
        else
        {
            sendError(transport, "unknown value");
            return;
        }
        transport.send(doc);
    }
}
