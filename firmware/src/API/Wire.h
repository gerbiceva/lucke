#pragma once
#include <string_view>
#include <ArduinoJson.h>
#include "API/API.h"

// The JSON shapes of protocol 2 (docs/api.md) for what the API returns.
// Separate from the stored formats, which keep their own keys so flash
// written by older firmware still loads.
namespace Wire
{
    // Changes on every boot. Lets a client tell a restarted device from one that
    // kept running (logs `since`, heartbeat).
    std::string_view bootId();

    void fixture(JsonObject obj, const FixtureInfo& info);
    void fixtures(JsonArray arr, const Handler::FixturesInfo& info);
    void inputs(JsonArray arr, const Handler::InputsInfo& info);
    // The three report switches, as in get_settings.
    void settings(JsonObject obj, const Config::Engine& config);
    // ssid comes from the settings, the rest from the live connection.
    void wifi(JsonObject obj, const Net::WifiInfo& info, const std::string& ssid);
    // The describe reply's data. Also the serial report.
    void describe(JsonObject obj, const DeviceInfo& info);
    void logs(JsonObject obj, const LogsInfo& info);
}
