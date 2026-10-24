#include "API.h"
#include <memory>
#include "Input/SacnReceiver.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"API"};
}

const char* toString(ApiError error)
{
    switch (error)
    {
        case ApiError::FixtureNotFound:    return "fixture not found";
        case ApiError::UniverseOutOfRange: return "universe must be 1-63999";
        case ApiError::LedCountOutOfRange: return "leds must be 1-1000";
    }
    return "unknown error";
}

void DeviceInfo::toJson(JsonObject& obj)
{
    settings.toJson(obj);
    obj.remove("password");

    JsonObject wifiObj = obj["wifi"].to<JsonObject>();
    wifi.toJson(wifiObj);

    JsonObject fixturesObj = obj["fixtures"].to<JsonObject>();
    fixtures.toJson(fixturesObj);

    JsonObject inputsObj = obj["inputs"].to<JsonObject>();
    inputs.toJson(inputsObj);
}

void LogsInfo::toJson(JsonObject& obj)
{
    obj["first"] = first;
    obj["next"] = next;
    obj["lost"] = lost;
    JsonArray arr = obj["lines"].to<JsonArray>();
    for (const std::string& line : lines)
    {
        arr.add(line);
    }
}

API::API(Services::SettingsStore& settings, Handler::FixtureHandler& fixtures, Handler::InputHandler& inputs,
         Net::Wifi& wifi, Services::Telemetry& telemetry, Services::OtaService& ota)
    : m_settings(settings), m_fixtures(fixtures), m_inputs(inputs), m_wifi(wifi), m_telemetry(telemetry),
      m_ota(ota)
{}

DeviceInfo API::describe() const
{
    return {settings(), wifi(), fixtures(), inputs()};
}

Config::Engine API::settings() const
{
    Config::Engine config = m_settings.snapshot();
    config.password.clear();
    return config;
}

Net::WifiInfo API::wifi() const
{
    return m_wifi.info();
}

Handler::FixturesInfo API::fixtures() const
{
    return m_fixtures.fixtures();
}

std::expected<FixtureInfo, ApiError> API::fixture(uint8_t id) const
{
    Fixture* fix = m_fixtures.get(id);
    if (!fix) return std::unexpected(ApiError::FixtureNotFound);
    return fix->info();
}

std::expected<JsonDocument, ApiError> API::fixtureConfig(uint8_t id) const
{
    Fixture* fix = m_fixtures.get(id);
    if (!fix) return std::unexpected(ApiError::FixtureNotFound);
    return fix->configJson();
}

std::expected<JsonDocument, ApiError> API::fixturePresets(uint8_t id) const
{
    Fixture* fix = m_fixtures.get(id);
    if (!fix) return std::unexpected(ApiError::FixtureNotFound);
    return fix->presetsJson();
}

std::expected<JsonDocument, ApiError> API::fixtureOutputs(uint8_t id) const
{
    Fixture* fix = m_fixtures.get(id);
    if (!fix) return std::unexpected(ApiError::FixtureNotFound);
    return fix->outputsJson();
}

Handler::InputsInfo API::inputs() const
{
    return m_inputs.inputs();
}

LogsInfo API::logs(uint32_t since) const
{
    const Utils::LogHistory& history = Utils::Log::history();
    if (since > history.nextSeq()) since = 0;

    LogsInfo info;
    char buf[Utils::LogHistory::MaxLine];
    uint32_t seq = since;
    while (auto line = history.read(seq, buf))
    {
        if (info.lines.empty())
        {
            info.first = seq;
            info.lost = seq - since;
        }
        info.lines.emplace_back(*line);
        ++seq;
    }
    info.next = seq;
    return info;
}

std::expected<FixtureUpdate, ApiError> API::setFixture(uint8_t id, const FixturePatch& patch, Origin from)
{
    if (!m_fixtures.exists(id)) return std::unexpected(ApiError::FixtureNotFound);
    if (patch.universe &&
        (*patch.universe < Input::SacnReceiver::c_minUniverse || *patch.universe > Input::SacnReceiver::c_maxUniverse))
    {
        return std::unexpected(ApiError::UniverseOutOfRange);
    }
    if (patch.leds && (*patch.leds == 0 || *patch.leds > Handler::FixtureHandler::c_maxLeds))
    {
        return std::unexpected(ApiError::LedCountOutOfRange);
    }

    bool changed = false;
    if (patch.universe) changed |= m_fixtures.setUniverse(id, *patch.universe);
    if (patch.address) changed |= m_fixtures.setAddress(id, *patch.address);
    if (patch.preset) changed |= m_fixtures.setPreset(id, *patch.preset);
    if (patch.name) changed |= m_fixtures.setName(id, *patch.name);
    if (patch.highlight) changed |= m_fixtures.highlight(id);

    // The strip is built with its count at boot, so a new one needs a restart.
    bool rebooting = false;
    if (patch.leds && *patch.leds != m_fixtures.get(id)->ledCount())
    {
        changed |= m_fixtures.setLedCount(id, *patch.leds);
        rebooting = true;
    }

    FixtureInfo info = m_fixtures.get(id)->info();
    if (changed)
    {
        s_log.info("fixture %u changed via %.*s: universe %u, address %u, preset %u, name '%s', highlighted %s", id,
                   static_cast<int>(from.size()), from.data(), info.config.universe, info.config.address,
                   info.config.selectedPreset, info.config.name.c_str(), info.config.highlighted ? "true" : "false");
    }
    if (rebooting)
    {
        s_log.warn("fixture %u: %u LEDs via %.*s, rebooting to apply", id, *patch.leds,
                   static_cast<int>(from.size()), from.data());
        m_settings.flush();
        runLater(1000, [] { ESP.restart(); });
    }
    return FixtureUpdate{std::move(info), changed, rebooting};
}

void API::setWifi(std::optional<std::string> ssid, std::optional<std::string> password, Origin from,
                  uint32_t delayMs)
{
    if (!ssid && !password) return;

    m_settings.update([&](Config::Engine& c) {
        if (ssid) c.ssid = std::move(*ssid);
        if (password) c.password = std::move(*password);
    });
    Config::Engine config = m_settings.snapshot();
    s_log.info("wifi settings changed via %.*s, ssid '%s'", static_cast<int>(from.size()), from.data(),
               config.ssid.c_str());

    runLater(delayMs, [this, ssid = config.ssid, password = config.password]() {
        m_wifi.reconnect(ssid, password);
    });
}

void API::setSerialReport(bool enable, Origin from)
{
    s_log.info("serial_report_task set to %s via %.*s", enable ? "true" : "false", static_cast<int>(from.size()),
               from.data());
    m_telemetry.setSerialReport(enable);
    m_settings.update([enable](Config::Engine& c) { c.serial_report_task = enable; });
}

void API::setWirelessReport(bool enable, Origin from)
{
    s_log.info("wireless_report_task set to %s via %.*s", enable ? "true" : "false", static_cast<int>(from.size()),
               from.data());
    m_telemetry.setWirelessReport(enable);
    m_settings.update([enable](Config::Engine& c) { c.wireless_report_task = enable; });
}

void API::setWifiAnimation(bool enable, Origin from)
{
    s_log.info("wifi_animation set to %s via %.*s", enable ? "true" : "false", static_cast<int>(from.size()),
               from.data());
    m_settings.update([enable](Config::Engine& c) { c.wifi_animation = enable; });
}

void API::reboot(Origin from, uint32_t delayMs)
{
    s_log.info("reboot requested via %.*s", static_cast<int>(from.size()), from.data());
    // Fixture records are written with a delay; don't lose the last change.
    m_settings.flush();
    runLater(delayMs, [] { ESP.restart(); });
}

void API::enableOta(uint32_t minutes, Origin from)
{
    s_log.warn("OTA enabled for %lu min via %.*s", static_cast<unsigned long>(minutes),
               static_cast<int>(from.size()), from.data());
    m_ota.enable(minutes);
}

void API::factoryReset(Origin from, uint32_t delayMs)
{
    s_log.warn("factory reset requested via %.*s", static_cast<int>(from.size()), from.data());
    m_settings.factoryReset();
    runLater(delayMs, [] { ESP.restart(); });
}

void API::runLater(uint32_t delayMs, std::function<void()> fn)
{
    struct Job
    {
        uint32_t delayMs;
        std::function<void()> fn;
    };

    auto* job = new Job{delayMs, std::move(fn)};
    auto entry = [](void* arg) {
        {
            std::unique_ptr<Job> job(static_cast<Job*>(arg));
            vTaskDelay(pdMS_TO_TICKS(job->delayMs));
            job->fn();
        }
        vTaskDelete(nullptr);
    };
    if (xTaskCreate(entry, "api_later", 4096, job, 1, nullptr) != pdPASS)
    {
        s_log.error("failed to create deferred task");
        delete job;
    }
}
