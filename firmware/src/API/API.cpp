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

API::API(Services::SettingsStore& settings, Handler::FixtureHandler& fixtures, Handler::InputHandler& inputs,
         Net::Wifi& wifi, Services::Telemetry& telemetry)
    : m_settings(settings), m_fixtures(fixtures), m_inputs(inputs), m_wifi(wifi), m_telemetry(telemetry)
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

std::expected<FixtureUpdate, ApiError> API::setFixture(uint8_t id, const FixturePatch& patch, Origin from)
{
    if (!m_fixtures.exists(id)) return std::unexpected(ApiError::FixtureNotFound);
    if (patch.universe &&
        (*patch.universe < Input::SacnReceiver::c_minUniverse || *patch.universe > Input::SacnReceiver::c_maxUniverse))
    {
        return std::unexpected(ApiError::UniverseOutOfRange);
    }

    bool changed = false;
    if (patch.universe) changed |= m_fixtures.setUniverse(id, *patch.universe);
    if (patch.address) changed |= m_fixtures.setAddress(id, *patch.address);
    if (patch.preset) changed |= m_fixtures.setPreset(id, *patch.preset);
    if (patch.name) changed |= m_fixtures.setName(id, *patch.name);
    if (patch.highlight) changed |= m_fixtures.highlight(id);

    FixtureInfo info = m_fixtures.get(id)->info();
    if (changed)
    {
        s_log.info("fixture {} changed via {}: universe {}, address {}, preset {}, name '{}'", id, from,
                   info.config.universe, info.config.address, info.config.selectedPreset, info.config.name);
    }
    return FixtureUpdate{std::move(info), changed};
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
    s_log.info("wifi settings changed via {}, ssid '{}'", from, config.ssid);

    runLater(delayMs, [this, ssid = config.ssid, password = config.password]() {
        m_wifi.reconnect(ssid, password);
    });
}

void API::setSerialReport(bool enable, Origin from)
{
    s_log.info("serial_report_task set to {} via {}", enable, from);
    m_telemetry.setSerialReport(enable);
    m_settings.update([enable](Config::Engine& c) { c.serial_report_task = enable; });
}

void API::setWirelessReport(bool enable, Origin from)
{
    s_log.info("wireless_report_task set to {} via {}", enable, from);
    m_telemetry.setWirelessReport(enable);
    m_settings.update([enable](Config::Engine& c) { c.wireless_report_task = enable; });
}

void API::setWifiAnimation(bool enable, Origin from)
{
    s_log.info("wifi_animation set to {} via {}", enable, from);
    m_settings.update([enable](Config::Engine& c) { c.wifi_animation = enable; });
}

void API::reboot(Origin from, uint32_t delayMs)
{
    s_log.info("reboot requested via {}", from);
    // Fixture records are written with a delay; don't lose the last change.
    m_settings.flush();
    runLater(delayMs, [] { ESP.restart(); });
}

void API::factoryReset(Origin from, uint32_t delayMs)
{
    s_log.warn("factory reset requested via {}", from);
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
