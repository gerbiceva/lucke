#pragma once
#include <string>
#include "Utils/Storage.h"

class FixtureConfig
{
    static uint8_t s_ID;
protected:
    Utils::Storage m_storage;
public:
    uint8_t m_ID = 0;
    uint8_t universe = 1U;
    uint16_t address = 0U;
    uint8_t selectedPreset = 0U;

    std::string name;

    FixtureConfig(std::string fixtureName);

    virtual void setName(const std::string& other) {
        name = other;
        m_storage.putString("name", name);
    }

    virtual void setUniverse(uint8_t universe) = 0;
    
    virtual void setAddress(uint16_t newAddress)
    {
        address = newAddress;
        m_storage.putUShort("address", address);
    }
    
    virtual void setPreset(uint8_t newPreset) = 0;
    // uint8_t
};
