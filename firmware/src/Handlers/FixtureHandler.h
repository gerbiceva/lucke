#pragma once
#include <vector>
#include <cstdint>
#include "Traits/Serializable.h"
#include "Traits/Deserializable.h"
// #include "Fixture/Fixture.h"
#include "Utils/Optional.h"
#include "Fixtures.h"

class Fixture;

namespace Handler
{
    class FixtureHandler : public Traits::Serializable, public Traits::Deserializable
    {
        std::vector<Fixture*> fixtures;
    public:
        // static void addFixture(Fixture* fixture);
    
        template<typename TFixture>
        Fixture* addFixture()
        {
            Fixture* fix = new TFixture();

            fixtures.push_back(fix);
            return fix;
        }
    
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
    };
}
