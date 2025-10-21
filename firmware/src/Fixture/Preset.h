#pragma once
#include <string>
#include <cstdint>

struct Preset
{
    std::string name;
};

struct Preset1D : Preset 
{
    uint16_t nGroups;
};

struct Preset2D : Preset
{
    uint8_t width;
    uint8_t height;
}