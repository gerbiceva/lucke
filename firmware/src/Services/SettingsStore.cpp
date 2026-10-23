#include "SettingsStore.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"

namespace
{
    constexpr const char* c_settingsKey = "settings";
}

namespace Services
{
    void SettingsStore::load()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_loaded) return;
        m_loaded = true;

        m_engineNvs.setPartition("engine");
        m_fixtureNvs.setPartition("fixtures");

        if (!m_engineNvs.isKey(c_settingsKey))
        {
            m_engineNvs.putString(c_settingsKey, m_engine.toString());
            return;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, m_engineNvs.getString(c_settingsKey));
        if (err)
        {
            Utils::Logger::printf("[SETTINGS] stored settings unreadable (%s), using defaults\n", err.c_str());
            return;
        }
        m_engine.fromJson(doc.as<JsonVariantConst>());
    }

    Config::Engine& SettingsStore::engine()
    {
        return m_engine;
    }

    const Config::Engine& SettingsStore::engine() const
    {
        return m_engine;
    }

    void SettingsStore::save()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_engineNvs.putString(c_settingsKey, m_engine.toString());
    }

    std::optional<std::string> SettingsStore::loadFixture(uint8_t id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = fixtureKey(id);
        if (!m_fixtureNvs.isKey(key)) return std::nullopt;
        return m_fixtureNvs.getString(key);
    }

    void SettingsStore::saveFixture(uint8_t id, const std::string& json)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_fixtureNvs.putString(fixtureKey(id), json);
    }

    void SettingsStore::factoryReset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_engineNvs.clear();
        m_fixtureNvs.clear();
        Utils::Logger::println("[SETTINGS] factory reset: NVS cleared");
    }

    std::string SettingsStore::fixtureKey(uint8_t id)
    {
        return "fixture" + std::to_string(id);
    }
}
