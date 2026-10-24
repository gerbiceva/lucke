#pragma once
#include <concepts>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include "Core/Config/EngineConfig.h"
#include "Utils/Storage.h"

namespace Services
{
    // The only code that reads or writes NVS. Holds the engine settings and
    // the per-fixture records.
    //
    // NVS isn't ready during global construction, so the partitions are
    // opened on the first load() (from setup()), not in the constructor.
    class SettingsStore
    {
    public:
        // Opens NVS and loads the engine settings, writing defaults if none
        // are stored. Idempotent: later calls do nothing.
        void load();

        // Unlocked view, for boot code that runs before the other tasks.
        [[nodiscard]] const Config::Engine& engine() const;

        // Copy of the engine settings, safe to take from any task.
        [[nodiscard]] Config::Engine snapshot() const;

        // Applies change to the engine settings and persists them, as one
        // step under the store lock.
        template <std::invocable<Config::Engine&> F>
        void update(F&& change)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            change(m_engine);
            m_engineNvs.putString(c_settingsKey, m_engine.toString());
        }

        [[nodiscard]] std::optional<std::string> loadFixture(uint8_t id);
        void saveFixture(uint8_t id, const std::string& json);

        // Erases all stored settings and fixture records. The caller reboots;
        // the next boot starts from defaults.
        void factoryReset();

    private:
        static constexpr const char* c_settingsKey = "settings";
        static std::string fixtureKey(uint8_t id);

        // Serializes NVS access and guards m_engine, which the report and
        // wifi tasks read while commands change it.
        mutable std::mutex m_mutex;
        bool m_loaded = false;

        Config::Engine m_engine;
        Utils::Storage m_engineNvs;
        Utils::Storage m_fixtureNvs;
    };
}
