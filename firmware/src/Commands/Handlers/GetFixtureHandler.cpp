#include "GetFixtureHandler.h"
#include <algorithm>
#include <array>
#include <string_view>

namespace
{
    // One readable field of {"request":"getfixture","id":..,"value": name}.
    struct Field
    {
        std::string_view name;
        void (*get)(JsonVariant out, const API& api, uint8_t id, const Config::Fixture& config);
    };

    constexpr std::array c_fields{
        Field{"universe", [](JsonVariant out, const API&, uint8_t, const Config::Fixture& c) { out.set(c.universe); }},
        Field{"address",  [](JsonVariant out, const API&, uint8_t, const Config::Fixture& c) { out.set(c.address); }},
        Field{"preset",   [](JsonVariant out, const API&, uint8_t, const Config::Fixture& c) { out.set(c.selectedPreset); }},
        Field{"name",     [](JsonVariant out, const API&, uint8_t, const Config::Fixture& c) { out.set(c.name); }},
        Field{"type",     [](JsonVariant out, const API&, uint8_t, const Config::Fixture& c) { out.set(c.type); }},
        Field{"leds",     [](JsonVariant out, const API&, uint8_t, const Config::Fixture& c) { out.set(c.numLeds); }},
        Field{"config",   [](JsonVariant out, const API& api, uint8_t id, const Config::Fixture&) { out.set(*api.fixtureConfig(id)); }},
        Field{"presets",  [](JsonVariant out, const API& api, uint8_t id, const Config::Fixture&) { out.set(*api.fixturePresets(id)); }},
        Field{"outputs",  [](JsonVariant out, const API& api, uint8_t id, const Config::Fixture&) { out.set(*api.fixtureOutputs(id)); }},
    };
}

namespace Commands
{
    GetFixtureHandler::GetFixtureHandler(const API& api)
        : m_api(api)
    {}

    Interface::Reply GetFixtureHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();

        // No id: every fixture.
        if (!req["id"].is<uint8_t>())
        {
            m_api.fixtures().toJson(obj);
            return doc;
        }

        const uint8_t id = req["id"].as<uint8_t>();
        auto fixture = m_api.fixture(id);
        if (!fixture)
            return std::unexpected(toString(fixture.error()));

        // No value: the whole fixture.
        if (!req["value"].is<const char*>())
        {
            fixture->toJson(obj);
            return doc;
        }

        // One field: {"req": field, "val": ...}
        const std::string_view field = req["value"].as<const char*>();
        const auto it = std::ranges::find(c_fields, field, &Field::name);
        if (it == c_fields.end())
            return std::unexpected("unknown value");

        doc.clear();
        doc["req"] = field;
        it->get(doc["val"].to<JsonVariant>(), m_api, id, fixture->config);
        return doc;
    }
}
