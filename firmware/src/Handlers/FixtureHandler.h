#pragma once
#include <vector>
#include <cstdint>
#include "Fixture/Fixture.h"

class Fixture;

class FixtureHandler
{
    static std::vector<Fixture*> fixtures;
public:
    static void addFixture(Fixture* fixture);

    template<typename TFixture>
    static Fixture* addFixture()
    {
        Fixture* fix = new TFixture();
        fixtures.push_back(fix);
        return fix;
    }

    template<typename TFixture>
    static Fixture* addFixture(std::string name, std::string type)
    {
        Fixture* fix = new TFixture(name, type);
        fixtures.push_back(fix);
        return fix;
    }

    // template<typename TFixture, typename>
    // static Fixture* addFixture()
    // {
    //     Fixture* fix = new TFixture();
    //     fixtures.push_back(fix);
    //     return fix;
    // }

    static Fixture* get(uint8_t index);
    static std::vector<Fixture*>& allFixtures();

    static void updateTask();
    static JsonDocument describe();
    // static JsonDocument toJson();
};