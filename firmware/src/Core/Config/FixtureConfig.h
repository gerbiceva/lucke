#pragma once
#include <cstdint>
#include <string>

namespace Config
{

struct Fixture
{
    static uint8_t s_ID;
    uint8_t m_ID = 0;

    uint8_t universe = 8U;
    uint16_t address = 0U;
    uint8_t selectedPreset = 0U;

    std::string name = "name";
    std::string type = "type";

    bool highlighted = false;
};

}