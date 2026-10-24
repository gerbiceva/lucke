#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
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
        void _addFixture(std::unique_ptr<Fixture> fix);
        void persistFixture(uint8_t id);

        // Re-derive the universe span from the fixture's current
        // address + footprint, dropping the old span (oldBase..oldBase+oldCount)
        // and subscribing the new one. Call after any change to
        // universe, address, or preset (footprint).
        void resubscribe(Fixture* fix, uint16_t oldBase, uint8_t oldCount);

    public:
        FixtureHandler(InputHandler& inputs, Services::SettingsStore& settings);

        template<typename TFixture>
        void addFixture()
        {
            _addFixture(std::make_unique<TFixture>());
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
