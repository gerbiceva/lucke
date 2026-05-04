#pragma once
#include <cstdint>
#include <string>

struct FixtureConfig
{
    uint8_t universe = 8U;
    uint16_t address = 0U;
    uint8_t selectedPreset = 0U;

    std::string name = "name";
    std::string type = "type";
};