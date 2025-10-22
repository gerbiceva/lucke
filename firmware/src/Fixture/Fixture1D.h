#pragma once
#include "Fixture.h"
#include <vector>
#include "Preset.h"

class Fixture1D : public Fixture 
{
    uint16_t m_numGroups = 1;
    std::vector<Preset1D> m_presets;
public:
    Fixture1D(std::string name, std::string type, uint16_t numLeds, std::vector<Preset1D> presets);

    void update() override;
    void setPreset(uint8_t newPreset) override;
    // std::string getJsonString() const;

    [[nodiscard]] JsonDocument presetsToJson() const override;

};
