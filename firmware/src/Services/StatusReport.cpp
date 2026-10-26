#include "StatusReport.h"
#include <ArduinoJson.h>

namespace Services
{
    StatusReport::StatusReport(const SettingsStore& settings, Net::Wifi& wifi,
                               Handler::FixtureManager& fixtures, Handler::InputManager& inputs)
        : m_settings(settings), m_wifi(wifi), m_fixtures(fixtures), m_inputs(inputs)
    {}

    std::string StatusReport::build()
    {
        JsonDocument doc;
        JsonObject settingsDoc = doc["settings"].to<JsonObject>();
        writeSettings(settingsDoc);

        JsonObject wifiDoc = doc["wifi"].to<JsonObject>();
        m_wifi.toJson(wifiDoc);

        JsonObject fixtureDoc = doc["fixture_handler"].to<JsonObject>();
        m_fixtures.toJson(fixtureDoc);

        JsonObject inputDoc = doc["input_handler"].to<JsonObject>();
        m_inputs.toJson(inputDoc);

        std::string out;
        serializeJson(doc, out);
        return out;
    }

    std::string StatusReport::settings()
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        writeSettings(obj);

        std::string out;
        serializeJson(doc, out);
        return out;
    }

    void StatusReport::writeSettings(JsonObject& obj)
    {
        // Runs on the report task, so read a locked copy. Config::Engine::toJson
        // is also the NVS format, so it must keep the password; strip it here
        // for anything that leaves the device.
        Config::Engine config = m_settings.snapshot();
        config.toJson(obj);
        obj.remove("password");
    }
}
