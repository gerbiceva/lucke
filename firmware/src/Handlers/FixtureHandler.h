#pragma once
#include <vector>
#include <cstdint>
#include "Traits/Serializable.h"
#include "Traits/Deserializable.h"
// #include "Fixture/Fixture.h"
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
            Utils::Logger::println("[FIXTURE HANDLER] Added new fixture");
            Fixture* fix = new TFixture();

            fixtures.push_back(fix);
            return fix;
        }
    
        Fixture* get(uint8_t index);
        std::vector<Fixture*>& allFixtures();

        void update();

        void fromJson(std::string json) override;
        void toJson(JsonObject& doc) override;
        void toJsonFull(JsonObject& doc) override;
    };
}
