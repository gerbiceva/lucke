//
// Created by bobi on 24. 10. 26.
//

#pragma once
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <ArduinoJson.h>
#include "Core/Config/EngineConfig.h"
#include "Core/Fixture.h"
#include "Handlers/FixtureHandler.h"
#include "Handlers/InputHandler.h"
#include "Network/Wifi.h"
#include "Services/SettingsStore.h"
#include "Services/Telemetry.h"
#include "Services/OtaService.h"

enum class ApiError : uint8_t
{
    FixtureNotFound,
    UniverseOutOfRange,
    LedCountOutOfRange
};

const char* toString(ApiError error);

// Who asked for a change, for the log: "serial", "192.168.0.5", ...
using Origin = std::string_view;

// Everything describe reports. Settings never carry the password.
struct DeviceInfo
{
    Config::Engine settings;
    Net::WifiInfo wifi;
    Handler::FixturesInfo fixtures;
    Handler::InputsInfo inputs;

    // Same layout as the describe reply.
    void toJson(JsonObject& obj);
};

// Fields to change on a fixture; unset ones are left alone.
struct FixturePatch
{
    std::optional<uint16_t> universe;
    std::optional<uint16_t> address;
    std::optional<uint8_t> preset;
    std::optional<std::string> name;
    // Strip LED count. Saved, then the device reboots to apply it.
    std::optional<uint16_t> leds;
    // All LEDs white while on. Not saved.
    std::optional<bool> highlight;
};

struct FixtureUpdate
{
    FixtureInfo fixture;
    bool changed = false;
    bool rebooting = false;   // a new LED count is applied by restarting
};

// Lines from the log history (Info and up), oldest first.
struct LogsInfo
{
    uint32_t first = 0;   // number of lines[0]
    uint32_t next = 0;    // pass as `since` to get only newer lines
    uint32_t lost = 0;    // lines after `since` already overwritten
    std::vector<std::string> lines;

    void toJson(JsonObject& obj);
};

// What the device can do, without JSON or transports. Command handlers,
// the serial shell, or anything else call this.
class API
{
public:
    API(Services::SettingsStore& settings, Handler::FixtureHandler& fixtures, Handler::InputHandler& inputs,
        Net::Wifi& wifi, Services::Telemetry& telemetry, Services::OtaService& ota);

    // Reads
    [[nodiscard]] DeviceInfo describe() const;
    [[nodiscard]] Config::Engine settings() const;
    [[nodiscard]] Net::WifiInfo wifi() const;
    [[nodiscard]] Handler::FixturesInfo fixtures() const;
    [[nodiscard]] std::expected<FixtureInfo, ApiError> fixture(uint8_t id) const;
    [[nodiscard]] std::expected<JsonDocument, ApiError> fixtureConfig(uint8_t id) const;
    [[nodiscard]] std::expected<JsonDocument, ApiError> fixturePresets(uint8_t id) const;
    [[nodiscard]] std::expected<JsonDocument, ApiError> fixtureOutputs(uint8_t id) const;
    [[nodiscard]] Handler::InputsInfo inputs() const;
    // Lines numbered >= since. A since from before a reboot starts over.
    [[nodiscard]] LogsInfo logs(uint32_t since = 0) const;

    // Writes
    std::expected<FixtureUpdate, ApiError> setFixture(uint8_t id, const FixturePatch& patch, Origin from);
    // Saves the credentials, then reconnects after delayMs so the caller
    // can still reply over the old link.
    void setWifi(std::optional<std::string> ssid, std::optional<std::string> password, Origin from,
                 uint32_t delayMs = 1000);
    void setSerialReport(bool enable, Origin from);
    void setWirelessReport(bool enable, Origin from);
    void setWifiAnimation(bool enable, Origin from);

    // Actions. Both restart after delayMs, so the caller can still reply.
    void reboot(Origin from, uint32_t delayMs = 1000);
    void factoryReset(Origin from, uint32_t delayMs = 1000);
    // Accepts a firmware update over WiFi for the next `minutes`.
    void enableOta(uint32_t minutes, Origin from);

private:
    // Runs fn on a short-lived task after delayMs.
    static void runLater(uint32_t delayMs, std::function<void()> fn);

    Services::SettingsStore& m_settings;
    Handler::FixtureHandler& m_fixtures;
    Handler::InputHandler& m_inputs;
    Net::Wifi& m_wifi;
    Services::Telemetry& m_telemetry;
    Services::OtaService& m_ota;
};
