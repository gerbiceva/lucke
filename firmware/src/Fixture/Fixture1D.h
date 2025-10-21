#pragma once
#include "Fixture.h"

class Fixture1D : public Fixture 
{
    uint16_t m_numGroups = 1;
    std::vector<Preset1D> presets;
public:
    Fixture1D(std::string name, std::string type, uint16_t numLeds);

    void update() override;
    void setPreset(uint8_t newPreset) override;
};
