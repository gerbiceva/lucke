#include "SetFixtureHandler.h"

namespace Commands
{
    SetFixtureHandler::SetFixtureHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply SetFixtureHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        if (!req["id"].is<uint8_t>())
            return std::unexpected("missing id");

        FixturePatch patch;
        if (req["universe"].is<uint16_t>()) patch.universe = req["universe"].as<uint16_t>();
        if (req["address"].is<uint16_t>()) patch.address = req["address"].as<uint16_t>();
        if (req["presetIndex"].is<uint8_t>()) patch.preset = req["presetIndex"].as<uint8_t>();
        if (req["name"].is<const char*>()) patch.name = req["name"].as<const char*>();
        if (req["leds"].is<uint16_t>()) patch.leds = req["leds"].as<uint16_t>();
        if (req["highlight"].is<bool>()) patch.highlight = req["highlight"].as<bool>();

        auto result = m_api.setFixture(req["id"].as<uint8_t>(), patch, ctx.from);
        if (!result)
            return std::unexpected(toString(result.error()));

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["status"] = result->rebooting ? "ok, rebooting" : result->changed ? "ok" : "no changes";
        result->fixture.toJson(obj);
        return doc;
    }
}
