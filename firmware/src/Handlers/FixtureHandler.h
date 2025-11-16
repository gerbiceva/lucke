#pragma once
#include <vector>
#include <cstdint>
#include "Fixture/Fixture.h"

class Fixture;

class FixtureHandler
{
    static std::vector<Fixture*> fixtures;
public:
    static void addFixture(Fixture* fixture)
    {
        fixtures.push_back(fixture);
    }

    template<typename TFixture>
    static Fixture* addFixture()
    {
        Fixture* fix = new TFixture();
        fixtures.push_back(fix);
        return fix;
    }

    static Fixture* get(uint8_t id)
    {
        return fixtures[id];
    }

    static std::vector<Fixture*>& vec()
    {
        return fixtures;
    }

    static void updateTask(void*);

    static void wirelessConfig(void*);
};