#pragma once
#include <string>
#include "Handlers/FixtureHandler.h"

struct FixtureConfig
{
    // uint8_t m_ID = 0;
    uint8_t universe = 1U;
    uint16_t address = 0U;
    uint8_t selectedPreset = 0U;

    std::string name;

    FixtureConfig(std::string fixtureName)
        : name(fixtureName)
    {
        // m_ID = Handler::Fixture::addFixtureConfig(this);
    }

    virtual void setName(const std::string& other) {
        name = other;
    }

    virtual void setUniverse(uint8_t universe) = 0;
    
    virtual void setAddress(uint16_t newAddress)
    {
        address = newAddress;
    }
    
    virtual void setPreset(uint8_t newPreset) = 0;
    // uint8_t
};
