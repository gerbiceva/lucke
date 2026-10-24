#include "DescribeHandler.h"

namespace Commands
{
    DescribeHandler::DescribeHandler(const Services::SettingsStore& settings, Handler::FixtureHandler& fixtures, Handler::InputHandler& inputs, Net::Wifi& wifi)
        : m_settings(settings), m_fixtures(fixtures), m_inputs(inputs), m_wifi(wifi)
    {}

    void DescribeHandler::handle(JsonDocument& req, Interface::Transport& transport)
    {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();

        // Config::Engine::toJson is also the NVS format and keeps the
        // password; never send it off the device.
        Config::Engine config = m_settings.snapshot();
        config.toJson(obj);
        obj.remove("password");

        JsonObject wifiObj = obj["wifi"].to<JsonObject>();
        m_wifi.toJson(wifiObj);

        JsonObject fixturesObj = obj["fixtures"].to<JsonObject>();
        m_fixtures.toJson(fixturesObj);

        JsonObject inputsObj = obj["inputs"].to<JsonObject>();
        m_inputs.toJson(inputsObj);

        std::string response;
        serializeJson(doc, response);
        transport.send(response);
    }
}
