#pragma once
#include <vector>
#include <cstdint>
// #include "Fixture/Fixture.h"
#include "Fixtures.h"

class Fixture;

namespace Handler
{
    class FixtureHandler
    {
        static std::vector<Fixture*> fixtures;
    public:
        // static void addFixture(Fixture* fixture);
    
        template<typename TFixture>
        static Fixture* addFixture()
        {
            Utils::Logger::println("[FIXTURE HANDLER] Added new fixture");
            Fixture* fix = new TFixture();

            fixtures.push_back(fix);
            return fix;
        }
    
        static Fixture* get(uint8_t index);
        static std::vector<Fixture*>& allFixtures();

        static void update();
        static JsonDocument describe();
    };
}
