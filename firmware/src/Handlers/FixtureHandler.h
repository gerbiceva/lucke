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
            // Utils::Logger::println("Fixture handler, addFixture");
            // sleep(5);
            Fixture* fix = new TFixture();
            // Utils::Logger::println("Fixture handler, addFixture");
            // sleep(3);

            fixtures.push_back(fix);
            return fix;
            // return nullptr;
        }
    
        // template<typename TFixture>
        // static Fixture* addFixture(std::string name, std::string type)
        // {
        //     Fixture* fix = new TFixture(name, type);
        //     fixtures.push_back(fix);
        //     return fix;
        // }
    
        static Fixture* get(uint8_t index);
        static std::vector<Fixture*>& allFixtures();
    
        static void updateTask();
        static JsonDocument describe();
    };
}
