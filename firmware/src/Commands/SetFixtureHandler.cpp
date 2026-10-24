#include "SetFixtureHandler.h"
#include "Input/SacnReceiver.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"CMD"};
}

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

        if (req["universe"].is<uint16_t>())
        {
            uint16_t uni = req["universe"];
            if (uni < Input::SacnReceiver::c_minUniverse || uni > Input::SacnReceiver::c_maxUniverse)
            {
                transport.send("{\"err\":\"universe must be 1-63999\"}");
                return;
            }
            changed |= m_fixtures.setUniverse(id, uni);
        }

        if (req["address"].is<uint16_t>())
        {
            changed |= m_fixtures.setAddress(id, req["address"]);
        }

        if (req["presetIndex"].is<uint8_t>())
        {
            changed |= m_fixtures.setPreset(id, req["presetIndex"]);
        }

        if (req["name"].is<const char*>())
        {
            changed |= m_fixtures.setName(id, req["name"].as<const char*>());
        }

        if (req["highlight"].is<bool>())
        {
            changed |= m_fixtures.highlight(id);
        }

        if (changed)
        {
            std::string request;
            serializeJson(req, request);
            s_log.info("fixture {} changed via {}: {}", id, transport.remoteAddress(), request);
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
