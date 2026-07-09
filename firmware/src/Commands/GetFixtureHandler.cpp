#include "GetFixtureHandler.h"

namespace Commands
{
    GetFixtureHandler::GetFixtureHandler(Handler::FixtureHandler& fixtures)
        : m_fixtures(fixtures)
    {}

    void GetFixtureHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        if (!req["id"].is<uint8_t>())
        {
            JsonDocument doc;
            JsonObject obj = doc.to<JsonObject>();
            m_fixtures.toJson(obj);
            std::string response;
            serializeJson(doc, response);
            transport.send(response);
            return;
        }

        uint8_t id = req["id"];
        Fixture* fix = m_fixtures.get(id);
        if (!fix)
        {
            transport.send("{\"err\":\"fixture not found\"}");
            return;
        }

        JsonDocument doc;

        if (req["value"].is<const char*>())
        {
            std::string field = req["value"].as<const char*>();

            if (field == "universe")
            {
                doc["req"] = "universe";
                doc["val"] = fix->getUniverse();
            }
            else if (field == "address")
            {
                doc["req"] = "address";
                doc["val"] = fix->getAddress();
            }
            else if (field == "preset")
            {
                doc["req"] = "preset";
                doc["val"] = fix->getSelectedPreset();
            }
            else if (field == "name")
            {
                doc["req"] = "name";
                doc["val"] = fix->getName();
            }
            else if (field == "type")
            {
                doc["req"] = "type";
                doc["val"] = fix->getType();
            }
            else if (field == "config")
            {
                doc["req"] = "config";
                doc["val"] = fix->configJson();
            }
            else if (field == "presets")
            {
                doc["req"] = "presets";
                doc["val"] = fix->presetsJson();
            }
            else if (field == "outputs")
            {
                doc["req"] = "outputs";
                doc["val"] = fix->outputsJson();
            }
            else
            {
                doc["err"] = "unknown value";
            }
        }
        else
        {
            JsonObject obj = doc.to<JsonObject>();
            fix->toJson(obj);
        }

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
