#pragma once
// #include "../Lamp.h"
// #include <unordered_map>
#include <cstdint>
#include <vector>
// #include "Fixture/FixtureConfig.h"

class Fixture;

namespace Handler
{
    class Fixture
    {
        static std::vector<Fixture*> m_fixtureConfigs;
    public:
        [[nodiscard]] static uint8_t addFixture(Fixture* ptr)
        {
            m_fixtureConfigs.push_back(ptr);
            return m_fixtureConfigs.size() - 1;
        }
    };
}