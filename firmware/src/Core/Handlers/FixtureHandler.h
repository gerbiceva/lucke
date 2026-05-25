#pragma once
#include <vector>
#include <cstdint>
#include "Traits/Serializable.h"
#include "Traits/Deserializable.h"
#include "Utils/Optional.h"
#include "Utils/Storage.h"
#include "Fixtures/All.h"



// class Fixture;

namespace Handler
{
    class FixtureHandler : public Traits::Serializable, public Traits::Deserializable
    {
        Utils::Storage m_storage;
        std::vector<Fixture*> fixtures;

        Fixture* configureAndAddFixture(Fixture* fix, bool factoryReset);
    public:
        FixtureHandler()
            : m_storage(Utils::Storage("fixtures"))
        {
            
        }

        void updateFixtureStorage(uint8_t fid);

        template<typename TFixture>
        Fixture* addFixture(bool factoryReset)
        {
            Fixture* fix = new TFixture();
            return configureAndAddFixture(fix, factoryReset);
        }

        template<typename TFixture, typename... Args>
        Fixture* addFixture(bool factoryReset, Args&&... args)
        {
            Fixture* fix = new TFixture(std::forward<Args>(args)...);
            return configureAndAddFixture(fix, factoryReset);
        }
    
        bool exists(uint8_t index);
        Fixture* get(uint8_t index);
        std::vector<Fixture*>& allFixtures();
        Utils::Optional<JsonDocument> fixtureJson(uint8_t id);

        bool fixtureExists(uint8_t id) const { return fixtures.size() > id; }
        Utils::Optional<std::string> setFixtureUniverse(uint8_t id, uint8_t universe);
        Utils::Optional<std::string> setFixtureAddress(uint8_t id, uint16_t address);
        Utils::Optional<std::string> setFixturePreset(uint8_t id, uint8_t preset);
        Utils::Optional<std::string> setFixtureName(uint8_t id, std::string name);
        Utils::Optional<std::string> highlightFixture(uint8_t id);

        void update();

        void fromJson(std::string json) override;
        void toJson(JsonObject& doc) override;
        void toJsonFull(JsonObject& doc) override;

        JsonDocument presetsJson();
    };
}
