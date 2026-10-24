#include "SetFixtureHandler.h"

namespace
{
    void send(Interface::Transport& transport, const JsonDocument& doc)
    {
        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }

    void sendError(Interface::Transport& transport, const char* err)
    {
        JsonDocument doc;
        doc["err"] = err;
        send(transport, doc);
    }
}

namespace Commands
{
    SetFixtureHandler::SetFixtureHandler(API& api)
        : m_api(api)
    {}

    void SetFixtureHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        if (!req["id"].is<uint8_t>())
        {
            sendError(transport, "missing id");
            return;
        }

        FixturePatch patch;
        if (req["universe"].is<uint16_t>()) patch.universe = req["universe"].as<uint16_t>();
        if (req["address"].is<uint16_t>()) patch.address = req["address"].as<uint16_t>();
        if (req["presetIndex"].is<uint8_t>()) patch.preset = req["presetIndex"].as<uint8_t>();
        if (req["name"].is<const char*>()) patch.name = req["name"].as<const char*>();
        if (req["leds"].is<uint16_t>()) patch.leds = req["leds"].as<uint16_t>();
        if (req["highlight"].is<bool>()) patch.highlight = req["highlight"].as<bool>();

        auto result = m_api.setFixture(req["id"], patch, transport.remoteAddress());
        if (!result)
        {
            sendError(transport, toString(result.error()));
            return;
        }

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["status"] = result->rebooting ? "ok, rebooting" : result->changed ? "ok" : "no changes";
        result->fixture.toJson(obj);
        send(transport, doc);
    }
}
