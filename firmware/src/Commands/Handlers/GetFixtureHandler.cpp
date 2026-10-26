#include "GetFixtureHandler.h"
#include "API/Wire.h"
#include "Commands/ApiReply.h"

namespace Commands
{
    GetFixtureHandler::GetFixtureHandler(const API& api)
        : m_api(api)
    {}

    Interface::Reply GetFixtureHandler::handle(const JsonDocument& req, const Interface::Context& ctx)
    {
        auto id = optInt(req, "id");
        if (!id) return std::unexpected(id.error());
        if (!*id || **id < 0 || **id > 255) return missing("id");

        // "include": any of "outputs" and "presets" adds that key to the fixture.
        bool outputs = false;
        bool presets = false;
        const JsonVariantConst include = req["include"];
        if (!include.isNull())
        {
            if (!include.is<JsonArrayConst>()) return wrongType("include", "an array");
            for (JsonVariantConst entry : include.as<JsonArrayConst>())
            {
                const std::string_view name = entry.is<const char*>() ? entry.as<const char*>() : "";
                if (name == "outputs") outputs = true;
                else if (name == "presets") presets = true;
                else return Interface::fail(Interface::Code::InvalidArg, "include has an unknown entry", "include");
            }
        }

        const uint8_t fixtureId = static_cast<uint8_t>(**id);
        auto fixture = m_api.fixture(fixtureId);
        if (!fixture) return fail(fixture.error());

        Interface::Success reply;
        JsonObject obj = reply.data.to<JsonObject>();
        Wire::fixture(obj, *fixture);

        if (outputs) obj["outputs"].set(*m_api.fixtureOutputs(fixtureId));
        if (presets) obj["presets"].set(*m_api.fixturePresets(fixtureId));
        return reply;
    }
}
