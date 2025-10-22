#pragma once
#include <string>
#include <cstdint>
#include "ArduinoJson.h"

struct Preset
{
    std::string name;

    Preset(const std::string& n)
        : name(n)
        {}
    [[nodiscard]] virtual JsonDocument toJson() const = 0;
};

struct Preset1D : Preset 
{
    uint16_t nGroups;

    Preset1D(const std::string& n, uint16_t numGroups)
        : Preset(n), nGroups(numGroups)
        {}

    [[nodiscard]] JsonDocument toJson() const override
    {
        JsonDocument doc;
        doc["description"] = name;
        doc["num_groups"] = nGroups;
        return doc;
    }
    
};

struct Preset2D : Preset
{
    uint8_t width;
    uint8_t height;

    Preset2D(const std::string& n, uint8_t w, uint8_t h)
        : Preset(n), width(w), height(h)
        {}

    [[nodiscard]] JsonDocument toJson() const override
    {
        JsonDocument doc;
        doc["description"] = name;
        doc["width"] = width;
        doc["height"] = height;
        return doc;
    }
};