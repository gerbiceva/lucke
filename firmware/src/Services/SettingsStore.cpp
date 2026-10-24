#include "SettingsStore.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"

namespace
{
    // How often the save task checks for queued records.
    constexpr TickType_t c_checkInterval = pdMS_TO_TICKS(200);
}

namespace Services
{
    void SettingsStore::load()
    {
        std::scoped_lock lock(m_mutex);
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

    void SettingsStore::start(Tasks::Manager& tasks)
    {
        tasks.spawnTask("settings_save", [this]() {
            saveLoop();
        }, 1, 4096);
    }

    const Config::Engine& SettingsStore::engine() const
    {
        return m_engine;
    }

    Config::Engine SettingsStore::snapshot() const
    {
        std::scoped_lock lock(m_mutex);
        return m_engine;
    }

    std::optional<std::string> SettingsStore::loadFixture(uint8_t id)
    {
        std::scoped_lock lock(m_mutex);
        if (auto it = m_pendingFixtures.find(id); it != m_pendingFixtures.end())
        {
            return it->second;
        }

        std::string key = fixtureKey(id);
        if (!m_fixtureNvs.isKey(key)) return std::nullopt;
        return m_fixtureNvs.getString(key);
    }

    void SettingsStore::saveFixture(uint8_t id, std::string json)
    {
        std::scoped_lock lock(m_mutex);
        m_pendingFixtures[id] = std::move(json);
        m_lastFixtureChange = xTaskGetTickCount();
    }

    void SettingsStore::flush()
    {
        std::scoped_lock lock(m_mutex);
        writePending();
    }

    void SettingsStore::factoryReset()
    {
        std::scoped_lock lock(m_mutex);
        // Otherwise the save task would write them back after the erase.
        m_pendingFixtures.clear();
        m_engineNvs.clear();
        m_fixtureNvs.clear();
        Utils::Logger::println("[SETTINGS] factory reset: NVS cleared");
    }

    void SettingsStore::saveLoop()
    {
        while (true)
        {
            vTaskDelay(c_checkInterval);

            std::scoped_lock lock(m_mutex);
            if (!m_pendingFixtures.empty() && xTaskGetTickCount() - m_lastFixtureChange >= c_saveDelay)
            {
                writePending();
            }
        }
    }

    void SettingsStore::writePending()
    {
        for (const auto& [id, json] : m_pendingFixtures)
        {
            m_fixtureNvs.putString(fixtureKey(id), json);
        }
        m_pendingFixtures.clear();
    }

    std::string SettingsStore::fixtureKey(uint8_t id)
    {
        return "fixture" + std::to_string(id);
    }
}
