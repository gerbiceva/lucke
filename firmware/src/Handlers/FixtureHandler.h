#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include "Core/Fixture.h"
#include "Handlers/InputHandler.h"
#include "Interfaces/Serializable.h"
#include "Interfaces/Deserializable.h"
#include "Utils/Storage.h"
#include "Utils/Logger.h"

namespace Handler
{
    class FixtureHandler : public Interface::Serializable, public Interface::Deserializable
    {
        void _addFixture(std::unique_ptr<Fixture> fix, bool factoryReset);
        void persistFixture(uint8_t id);

        // Re-derive the universe span from the fixture's current
        // address + footprint, dropping the old span (oldBase..oldBase+oldCount)
        // and subscribing the new one. Call after any change to
        // universe, address, or preset (footprint).
        void resubscribe(Fixture* fix, uint8_t oldBase, uint8_t oldCount);

    public:
        FixtureHandler(InputHandler& inputs);

        template<typename TFixture>
        void addFixture(bool factoryReset)
        {
            _addFixture(std::make_unique<TFixture>(), factoryReset);
        }

        Fixture* get(uint8_t id);
        const std::vector<std::unique_ptr<Fixture>>& all() const;
        bool exists(uint8_t id);

        bool setUniverse(uint8_t id, uint8_t uni);
        bool setAddress(uint8_t id, uint16_t addr);
        bool setPreset(uint8_t id, uint8_t preset);
        bool setName(uint8_t id, const std::string& name);
        bool highlight(uint8_t id);

        void update();
        void wifiAnimation();

        void fromJson(std::string json) override;
        void toJson(JsonObject& obj) override;
        JsonDocument toJsonDoc() override;

    private:
        InputHandler& m_inputs;
        Utils::Storage m_storage;
        bool m_storageReady = false;
        std::vector<std::unique_ptr<Fixture>> m_fixtures;
        uint8_t m_nextId = 0;
    };
}
