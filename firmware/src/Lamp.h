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
    std::string m_Name = "<empty>";
    std::string m_Type;
    std::vector<DMXPreset> presets;
    uint8_t* ledBuffer;

    Lamp(std::string type, unsigned int nLeds, std::vector<DMXPreset> pres) 
        : m_Type(type), numLeds(nLeds), presets(pres) {
        m_Name = type;
        ledBuffer = new uint8_t[getLedSize()]();
    }

    Lamp(std::string type, unsigned int nLeds, std::vector<DMXPreset> pres, std::string name) 
        : m_Type(type), numLeds(nLeds), presets(pres), m_Name(name) {
        ledBuffer = new uint8_t[getLedSize()]();
    }

    std::string toString() { return m_Type + ": " + m_Name; };
    constexpr unsigned int getLedSize() { return numLeds * numPxls; }
};
