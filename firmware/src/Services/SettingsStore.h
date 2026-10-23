#pragma once
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

        [[nodiscard]] Config::Engine& engine();
        [[nodiscard]] const Config::Engine& engine() const;

        // Persists the engine settings.
        void save();

        [[nodiscard]] std::optional<std::string> loadFixture(uint8_t id);
        void saveFixture(uint8_t id, const std::string& json);

        // Erases all stored settings and fixture records. The caller reboots;
        // the next boot starts from defaults.
        void factoryReset();

    private:
        static std::string fixtureKey(uint8_t id);

        // Serializes NVS access: serial and TCP command tasks can save at the
        // same time.
        std::mutex m_mutex;
        bool m_loaded = false;

        Config::Engine m_engine;
        Utils::Storage m_engineNvs;
        Utils::Storage m_fixtureNvs;
    };
}
