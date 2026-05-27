#include "SetFixtureHandler.h"

namespace Commands
{
    SetFixtureHandler::SetFixtureHandler(Handler::FixtureHandler& fixtures)
        : m_fixtures(fixtures)
    {}

    void SetFixtureHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        if (!req["id"].is<uint8_t>())
        {
            transport.send("{\"err\":\"missing id\"}");
            return;
        }

        uint8_t id = req["id"];
        if (!m_fixtures.exists(id))
        {
            transport.send("{\"err\":\"fixture not found\"}");
            return;
        }

        bool changed = false;

        if (req["universe"].is<uint8_t>())
        {
            m_fixtures.setUniverse(id, req["universe"]);
            changed = true;
        }

        if (req["address"].is<uint16_t>())
        {
            m_fixtures.setAddress(id, req["address"]);
            changed = true;
        }

        if (req["presetIndex"].is<uint8_t>())
        {
            m_fixtures.setPreset(id, req["presetIndex"]);
            changed = true;
        }

        if (req["name"].is<const char*>())
        {
            m_fixtures.setName(id, req["name"].as<const char*>());
            changed = true;
        }

        if (req["highlight"].is<bool>())
        {
            m_fixtures.highlight(id);
            changed = true;
        }

        Fixture* fix = m_fixtures.get(id);
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["status"] = changed ? "ok" : "no changes";
        fix->toJson(obj);

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
