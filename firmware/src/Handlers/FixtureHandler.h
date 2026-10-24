#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <optional>
#include <type_traits>
#include "Core/Fixture.h"
#include "Handlers/InputHandler.h"
#include "Interfaces/Serializable.h"
#include "Interfaces/Deserializable.h"
#include "Services/SettingsStore.h"
#include "Utils/Logger.h"

namespace Handler
{
    struct FixturesInfo
    {
        std::vector<FixtureInfo> fixtures;

        void toJson(JsonObject& obj)
        {
            JsonArray arr = obj["fixtures"].to<JsonArray>();
            for (FixtureInfo& fix : fixtures)
            {
                JsonObject entry = arr.add<JsonObject>();
                fix.toJson(entry);
            }
        }
    };

    class FixtureHandler : public Interface::Serializable, public Interface::Deserializable
    {
        void _addFixture(std::unique_ptr<Fixture> fix, const std::optional<std::string>& stored);
        // The record for the fixture about to be added, if any.
        std::optional<std::string> loadStored();
        // "num_leds" from a stored record, or 0 if there is none.
        static uint16_t storedLedCount(const std::optional<std::string>& stored);
        void persistFixture(uint8_t id);

        // Resubscribes to the universes the fixture now spans. Call after any
        // universe, address or preset change.
        void resubscribe(Fixture* fix, uint16_t oldBase, uint8_t oldCount);

    public:
        FixtureHandler(InputHandler& inputs, Services::SettingsStore& settings);

        static constexpr uint16_t c_maxLeds = 1000;

        template<typename TFixture>
            requires std::is_default_constructible_v<TFixture>
        void addFixture()
        {
            const auto stored = loadStored();
            _addFixture(std::make_unique<TFixture>(), stored);
        }

        // A strip: built with its stored LED count, or defaultLeds until one
        // is stored (setfixture "leds").
        template<typename TFixture>
            requires std::is_constructible_v<TFixture, uint16_t>
        void addFixture(uint16_t defaultLeds)
        {
            const auto stored = loadStored();
            const uint16_t leds = storedLedCount(stored);
            _addFixture(std::make_unique<TFixture>(leds ? leds : defaultLeds), stored);
        }

        [[nodiscard]] Fixture* get(uint8_t id);
        [[nodiscard]] const std::vector<std::unique_ptr<Fixture>>& all() const;
        [[nodiscard]] bool exists(uint8_t id);
        [[nodiscard]] FixturesInfo fixtures() const;

        // Return false if no fixture has this id.
        [[nodiscard]] bool setUniverse(uint8_t id, uint16_t uni);
        [[nodiscard]] bool setAddress(uint8_t id, uint16_t addr);
        [[nodiscard]] bool setPreset(uint8_t id, uint8_t preset);
        [[nodiscard]] bool setName(uint8_t id, const std::string& name);
        [[nodiscard]] bool highlight(uint8_t id);
        // Saves a new LED count and resets the preset to 0. Applied after a reboot.
        [[nodiscard]] bool setLedCount(uint8_t id, uint16_t leds);

        void update();
        void wifiAnimation();

        void fromJson(const std::string& json) override;
        void toJson(JsonObject& obj) override;
        JsonDocument toJsonDoc() override;

    private:
        InputHandler& m_inputs;
        Services::SettingsStore& m_settings;
        std::vector<std::unique_ptr<Fixture>> m_fixtures;
        uint8_t m_nextId = 0;
    };
}
