#pragma once
#include "Config.h"
#include <string>
#include <vector>


struct Lamp {
    struct DMXPreset {
        uint16_t numOfGroups;
        std::string description;
    
        // DMXPreset(uint16_t numberOfGroups, std::string description = "") : numOfGroups(numberOfGroups), description(description) {}
    };

    unsigned int numLeds;
    uint8_t numPxls = 3;
    std::string name = "<empty>";
    std::string type;
    std::vector<DMXPreset> presets;
    uint8_t* ledBuffer;

    Lamp(std::string typ, unsigned int nLeds, std::vector<DMXPreset> pres) 
        : type(typ), numLeds(nLeds), presets(pres) {
        name = typ;
        ledBuffer = new uint8_t[getLedSize()]();
    }

    std::string toString() { return type + ": " + name; };
    constexpr unsigned int getLedSize() { return numLeds * numPxls; }
};
