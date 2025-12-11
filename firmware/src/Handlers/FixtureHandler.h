#pragma once
#include <vector>
#include <cstdint>
#include "Traits/Serializable.h"
// #include "Fixture/Fixture.h"
#include "Fixtures.h"

class Fixture;

namespace Handler
{
    class FixtureHandler : public Traits::Serializable
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
        JsonDocument describe() override;
    };
}
