#include "SetFixtureHandler.h"
#include "API/Wire.h"
#include "Commands/ApiReply.h"

namespace Commands
{
    SetFixtureHandler::SetFixtureHandler(API& api)
        : m_api(api)
    {}

    Interface::Reply SetFixtureHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        auto id = optInt(req, "id");
        if (!id) return std::unexpected(id.error());
        if (!*id || **id < 0 || **id > 255) return missing("id");

        FixturePatch patch;

        auto universe = optInt(req, "universe");
        auto address = optInt(req, "address");
        auto preset = optInt(req, "preset_index");
        auto name = optString(req, "name");
        auto highlight = optBool(req, "highlight");
        auto output = optInt(req, "output");
        if (!universe) return std::unexpected(universe.error());
        if (!address) return std::unexpected(address.error());
        if (!preset) return std::unexpected(preset.error());
        if (!name) return std::unexpected(name.error());
        if (!highlight) return std::unexpected(highlight.error());
        if (!output) return std::unexpected(output.error());
        patch.universe = *universe;
        patch.address = *address;
        patch.preset = *preset;
        patch.name = std::move(*name);
        patch.highlight = *highlight;
        patch.output = *output;

        const JsonVariantConst settings = req["settings"];
        if (!settings.isNull())
        {
            if (!settings.is<JsonObjectConst>()) return wrongType("settings", "an object");
            if (!patch.output) return missing("output");
            patch.settings.set(settings);
        }
        else if (patch.output)
        {
            return missing("settings");
        }

        auto result = m_api.setFixture(static_cast<uint8_t>(**id), patch, ctx.from);
        if (!result) return fail(result.error());

        Interface::Success reply;
        reply.rebooting = result->rebooting;
        JsonObject data = reply.data.to<JsonObject>();
        data["changed"] = result->changed;
        JsonObject fixture = data["fixture"].to<JsonObject>();
        Wire::fixture(fixture, result->fixture);
        // The client sent settings: show what they are now.
        if (patch.output) fixture["outputs"].set(*m_api.fixtureOutputs(static_cast<uint8_t>(**id)));
        return reply;
    }
}
