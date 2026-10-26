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
#include "Core/Managers/FixtureManager.h"
#include "Core/Managers/InputManager.h"
#include "Network/Wifi.h"
#include "Services/SettingsStore.h"
#include "Services/Telemetry.h"
#include "Services/OtaService.h"

// Why a call failed. `code` is one of the codes in docs/apiv2.md (a
// literal); `arg` names the request argument at fault ("universe",
// "settings.invert", ...).
struct ApiError
{
    std::string_view code;
    std::string arg;
    std::string msg;

    static ApiError notFound(std::string arg, std::string msg)
    {
        return {"not_found", std::move(arg), std::move(msg)};
    }
    static ApiError invalidArg(std::string arg, std::string msg)
    {
        return {"invalid_arg", std::move(arg), std::move(msg)};
    }
    static ApiError outOfRange(std::string arg, std::string msg)
    {
        return {"out_of_range", std::move(arg), std::move(msg)};
    }
    static ApiError forbidden(std::string arg, std::string msg)
    {
        return {"forbidden", std::move(arg), std::move(msg)};
    }
};

// Who asked for a change, for the log: "serial", "192.168.0.5", ...
using Origin = std::string_view;

// Everything describe reports. Settings never carry the password.
struct DeviceInfo
{
    Config::Engine settings;
    Net::WifiInfo wifi;
    Handler::FixturesInfo fixtures;
    Handler::InputsInfo inputs;
};

// Fields to change on a fixture; unset ones are left alone. Numbers are wide
// so setFixture can reject out-of-range values instead of the caller wrapping them.
struct FixturePatch
{
    std::optional<int32_t> universe;
    std::optional<int32_t> address;
    std::optional<int32_t> preset;
    std::optional<std::string> name;
    // Output settings: `settings` (key -> value) for output `output`. Both set or
    // both unset; the caller checks that.
    std::optional<int32_t> output;
    JsonDocument settings;
    // All LEDs white while on. Not saved.
    std::optional<bool> highlight;
};

struct FixtureUpdate
{
    FixtureInfo fixture;
    bool changed = false;
    bool rebooting = false;   // a requiresReboot setting is applied by restarting
};

// Lines from the log history (Info and up), oldest first.
struct LogsInfo
{
    uint32_t first = 0;   // number of lines[0]
    uint32_t next = 0;    // pass as `since` to get only newer lines
    uint32_t lost = 0;    // lines after `since` already overwritten
    bool reset = false;   // `since` was from another boot and was ignored
    std::vector<std::string> lines;
};

// What the device can do, without JSON or transports. Command handlers,
// the serial shell, or anything else call this.
class API
{
public:
    API(Services::SettingsStore& settings, Handler::FixtureManager& fixtures, Handler::InputManager& inputs,
        Net::Wifi& wifi, Services::Telemetry& telemetry, Services::OtaService& ota);

    // Reads
    [[nodiscard]] DeviceInfo describe() const;
    [[nodiscard]] Config::Engine settings() const;
    [[nodiscard]] Net::WifiInfo wifi() const;
    [[nodiscard]] Handler::FixturesInfo fixtures() const;
    [[nodiscard]] std::expected<FixtureInfo, ApiError> fixture(uint8_t id) const;
    // Array of {name, settings: [...]}, index = preset index.
    [[nodiscard]] std::expected<JsonDocument, ApiError> fixturePresets(uint8_t id) const;
    // Array, one entry per output.
    [[nodiscard]] std::expected<JsonDocument, ApiError> fixtureOutputs(uint8_t id) const;
    [[nodiscard]] Handler::InputsInfo inputs() const;
    // Lines numbered >= since. If bootId is given and isn't this boot's, since
    // is from before a reboot: it is ignored and the result has reset set.
    [[nodiscard]] LogsInfo logs(uint32_t since = 0, std::string_view bootId = {}) const;

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
    Handler::FixtureManager& m_fixtures;
    Handler::InputManager& m_inputs;
    Net::Wifi& m_wifi;
    Services::Telemetry& m_telemetry;
    Services::OtaService& m_ota;
};
