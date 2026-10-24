#pragma once
#include <concepts>
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include "Core/Config/EngineConfig.h"
#include "Tasks/TaskExecutor.h"
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

        // Spawns the task that writes queued fixture records. Call once,
        // after load().
        void start(Tasks::Manager& tasks);

        // Unlocked view, for boot code that runs before the other tasks.
        [[nodiscard]] const Config::Engine& engine() const;

        // Copy of the engine settings, safe to take from any task.
        [[nodiscard]] Config::Engine snapshot() const;

        // Applies change to the engine settings and persists them, as one
        // step under the store lock.
        template <std::invocable<Config::Engine&> F>
        void update(F&& change)
        {
            std::scoped_lock lock(m_mutex);
            change(m_engine);
            if (!m_wiped) m_engineNvs.putString(c_settingsKey, m_engine.toString());
        }

        [[nodiscard]] std::optional<std::string> loadFixture(uint8_t id);

        // Queues a fixture record. It is written once no fixture has changed
        // for c_saveDelay, so dragging a slider in a UI costs one flash
        // write, not one per step.
        void saveFixture(uint8_t id, std::string json);

        // Writes queued fixture records now. Call before restarting.
        void flush();

        // Erases all stored settings and fixture records, and drops queued
        // ones. Nothing is written after this, so a change that lands before
        // the caller reboots can't survive the reset; the next boot starts
        // from defaults.
        void factoryReset();

    private:
        static constexpr const char* c_settingsKey = "settings";
        static constexpr TickType_t c_saveDelay = pdMS_TO_TICKS(1000);
        static std::string fixtureKey(uint8_t id);

        void saveLoop();
        // Caller holds m_mutex.
        void writePending();

        // Serializes NVS access and guards m_engine (read by the report and
        // wifi tasks while commands change it) and m_pendingFixtures.
        mutable std::mutex m_mutex;
        bool m_loaded = false;
        bool m_wiped = false;   // set by factoryReset(), until reboot

        Config::Engine m_engine;
        Utils::Storage m_engineNvs;
        Utils::Storage m_fixtureNvs;

        std::map<uint8_t, std::string> m_pendingFixtures;
        TickType_t m_lastFixtureChange = 0;
    };
}
