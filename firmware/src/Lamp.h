#pragma once
#include "Config.h"
#include "Grid.h"
#include <string>
#include <vector>


struct Lamp {
    struct DMXPreset {
        uint16_t wsize;
        uint16_t hsize;
        std::string description;
    
        DMXPreset(uint16_t groupBy, std::string description) : wsize(groupBy), hsize(1), description(description) {}
        DMXPreset(uint16_t wsize, uint16_t hsize, std::string description) : wsize(wsize), hsize(hsize), description(description) {}
        // DMXPreset(uint16_t numberOfGroups, std::string description = "") : numOfGroups(numberOfGroups), description(description) {}
    };

    Grid grid;

    unsigned int numLeds;
    uint8_t numPxls = 3;
    std::string m_Name = "<empty>";
    std::string m_Type;
    std::vector<DMXPreset> presets;
    uint8_t* ledBuffer;

    // Lamp(std::string type, unsigned int nLeds, std::vector<DMXPreset> pres) 
    //     : m_Type(type), numLeds(nLeds), presets(pres) {
    //     m_Name = type;
    //     ledBuffer = new uint8_t[getLedSize()]();
    // }

    Lamp(std::string type, unsigned int nLeds, std::vector<DMXPreset> pres) 
        : m_Type(type), numLeds(nLeds), presets(pres), grid(nLeds, 1, 1, 1) {
        m_Name = type;
        ledBuffer = new uint8_t[getLedSize()]();
    }

    Lamp(std::string type, unsigned int nLeds, std::vector<DMXPreset> pres, std::string name) 
        : m_Type(type), numLeds(nLeds), presets(pres), m_Name(name), grid(nLeds, 1, 1, 1) {
        ledBuffer = new uint8_t[getLedSize()]();
    }

    Lamp(std::string type, unsigned int nLeds, int width, int height, int wsize, int hsize, std::vector<DMXPreset> pres, std::string name) 
        : m_Type(type), numLeds(nLeds), presets(pres), m_Name(name), grid(width, height, wsize, hsize) {
        ledBuffer = new uint8_t[getLedSize()]();
    }

    void updateSubpixelSize(uint8_t presetIndex) {
        grid.updateSubpixelSize(presets[presetIndex].wsize, presets[presetIndex].hsize);
    }

    std::string toString() { return m_Type + ": " + m_Name; };
    constexpr unsigned int getLedSize() { return numLeds * numPxls; }
};
