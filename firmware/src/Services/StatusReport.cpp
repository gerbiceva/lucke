#include "StatusReport.h"
#include <ArduinoJson.h>
#include "API/Wire.h"

namespace Services
{
    StatusReport::StatusReport(const SettingsStore& settings, Net::Wifi& wifi,
                               Handler::FixtureManager& fixtures, Handler::InputManager& inputs)
        : m_settings(settings), m_wifi(wifi), m_fixtures(fixtures), m_inputs(inputs)
    {}

    std::string StatusReport::build()
    {
        // Runs on the report task, so read a locked copy of the settings.
        Config::Engine config = m_settings.snapshot();
        config.password.clear();

        JsonDocument doc;
        Wire::describe(doc.to<JsonObject>(),
                       DeviceInfo{std::move(config), m_wifi.info(), m_fixtures.fixtures(), m_inputs.inputs()});

        std::string out;
        serializeJson(doc, out);
        return out;
    }

    std::string StatusReport::settings()
    {
        const Config::Engine config = m_settings.snapshot();

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["version"] = ENGINE_VERSION;
        obj["protocol"] = PROTOCOL_VERSION;
        Wire::settings(obj["settings"].to<JsonObject>(), config);
        obj["ssid"] = config.ssid;

        std::string out;
        serializeJson(doc, out);
        return out;
    }
}
