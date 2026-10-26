#include "API.h"
#include <algorithm>
#include <memory>
#include <string>
#include "Input/SacnReceiver.h"
#include "API/Wire.h"
#include "Utils/Logger.h"

namespace
{
    constexpr Utils::Log s_log{"API"};

    // Last DMX channel a fixture can start at (512 channels per universe, 0-based).
    constexpr int32_t c_maxAddress = 511;
}

namespace
{
    using Interface::SettingDesc;

    ApiError noFixture()
    {
        return ApiError::notFound("id", "fixture not found");
    }

    // Checks one `settings` entry against the output's descriptor.
    std::expected<void, ApiError> checkSetting(const Interface::Output& output, std::string_view key,
                                               JsonVariantConst value)
    {
        const std::string arg = "settings." + std::string(key);

        const auto settings = output.settings();
        const auto it = std::ranges::find(settings, key, &SettingDesc::key);
        if (it == settings.end())
            return std::unexpected(ApiError::invalidArg(arg, "unknown setting"));
        if (!it->writable)
            return std::unexpected(ApiError::forbidden(arg, "setting is read-only"));

        switch (it->type)
        {
            case SettingDesc::Type::Bool:
                if (!value.is<bool>())
                    return std::unexpected(ApiError::invalidArg(arg, std::string(key) + " must be true or false"));
                break;
            case SettingDesc::Type::Int:
            {
                if (!value.is<int32_t>())
                    return std::unexpected(ApiError::invalidArg(arg, std::string(key) + " must be an integer"));
                const int32_t v = value.as<int32_t>();
                if (v < it->min || v > it->max)
                {
                    return std::unexpected(ApiError::outOfRange(
                        arg, std::string(key) + " must be " + std::to_string(it->min) + "-" + std::to_string(it->max)));
                }
                break;
            }
            case SettingDesc::Type::Enum:
            {
                const std::string_view v = value.is<const char*>() ? value.as<const char*>() : std::string_view{};
                if (!std::ranges::contains(it->options, v))
                    return std::unexpected(ApiError::invalidArg(arg, std::string(key) + " is not one of its options"));
                break;
            }
        }
        return {};
    }
}

API::API(Services::SettingsStore& settings, Handler::FixtureManager& fixtures, Handler::InputManager& inputs,
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
    if (!fix) return std::unexpected(noFixture());
    return fix->info();
}

std::expected<JsonDocument, ApiError> API::fixturePresets(uint8_t id) const
{
    Fixture* fix = m_fixtures.get(id);
    if (!fix) return std::unexpected(noFixture());
    return fix->presetsJson();
}

std::expected<JsonDocument, ApiError> API::fixtureOutputs(uint8_t id) const
{
    Fixture* fix = m_fixtures.get(id);
    if (!fix) return std::unexpected(noFixture());
    return fix->outputsJson();
}

Handler::InputsInfo API::inputs() const
{
    return m_inputs.inputs();
}

LogsInfo API::logs(uint32_t since, std::string_view bootId) const
{
    const Utils::LogHistory& history = Utils::Log::history();

    LogsInfo info;
    if ((!bootId.empty() && bootId != Wire::bootId()) || since > history.nextSeq())
    {
        since = 0;
        info.reset = true;
    }

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
    Fixture* fix = m_fixtures.get(id);
    if (!fix) return std::unexpected(noFixture());

    // Check everything first, so a bad argument leaves the fixture untouched.
    if (patch.universe &&
        (*patch.universe < Input::SacnReceiver::c_minUniverse || *patch.universe > Input::SacnReceiver::c_maxUniverse))
    {
        return std::unexpected(ApiError::outOfRange("universe", "universe must be 1-63999"));
    }
    if (patch.address && (*patch.address < 0 || *patch.address > c_maxAddress))
    {
        return std::unexpected(ApiError::outOfRange("address", "address must be 0-511"));
    }
    if (patch.preset && (*patch.preset < 0 || *patch.preset >= fix->getNumPresets()))
    {
        return std::unexpected(ApiError::outOfRange("preset_index", "preset_index out of range"));
    }
    if (patch.output)
    {
        if (*patch.output < 0 || static_cast<size_t>(*patch.output) >= fix->getOutputs().size())
            return std::unexpected(ApiError::notFound("output", "no such output"));

        const Interface::Output& output = *fix->getOutputs()[*patch.output];
        for (JsonPairConst kv : patch.settings.as<JsonObjectConst>())
        {
            if (auto ok = checkSetting(output, kv.key().c_str(), kv.value()); !ok)
                return std::unexpected(std::move(ok.error()));
        }
    }

    // The manager setters only say whether the fixture exists, so compare
    // before and after to tell whether anything really changed.
    const Config::Fixture before = fix->info().config;
    if (patch.universe) (void)m_fixtures.setUniverse(id, *patch.universe);
    if (patch.address) (void)m_fixtures.setAddress(id, *patch.address);
    if (patch.preset) (void)m_fixtures.setPreset(id, *patch.preset);
    if (patch.name) (void)m_fixtures.setName(id, *patch.name);
    if (patch.highlight) (void)m_fixtures.setHighlight(id, *patch.highlight);
    const Config::Fixture after = fix->info().config;
    bool changed = before.universe != after.universe || before.address != after.address ||
                   before.selectedPreset != after.selectedPreset || before.name != after.name ||
                   before.highlighted != after.highlighted;

    // A requiresReboot setting (a strip's LED count) is read when the output is
    // built, so it only takes effect after a restart.
    bool rebooting = false;
    if (patch.output)
    {
        const size_t output = *patch.output;
        for (JsonPairConst kv : patch.settings.as<JsonObjectConst>())
        {
            const std::string_view key = kv.key().c_str();
            if (!m_fixtures.setOutputSetting(id, output, key, kv.value())) continue;

            changed = true;
            const auto settings = fix->getOutputs()[output]->settings();
            const auto desc = std::ranges::find(settings, key, &SettingDesc::key);
            rebooting |= desc != settings.end() && desc->requiresReboot;
        }
    }

    // The current presets are for the old size; 0 ("group by 1") exists for every size.
    if (rebooting) (void)m_fixtures.setPreset(id, 0);

    FixtureInfo info = fix->info();
    if (changed)
    {
        s_log.info("fixture %u changed via %.*s: universe %u, address %u, preset %u, name '%s', highlighted %s", id,
                   static_cast<int>(from.size()), from.data(), info.config.universe, info.config.address,
                   info.config.selectedPreset, info.config.name.c_str(), info.config.highlighted ? "true" : "false");
    }
    if (rebooting)
    {
        s_log.warn("fixture %u: output settings via %.*s need a restart, rebooting", id,
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
