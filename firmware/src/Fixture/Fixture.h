#pragma once
#include "Utils/HardwareLED.h"
#include "Handlers/SacnHandler.h"
#include "FixtureConfig.h"
#include "Preset.h"

class Fixture : public Utils::HardwareLED, public FixtureConfig
{
    static uint8_t s_ID;
    uint8_t m_ID = 0U;
    std::string m_type;
protected:
	uint16_t m_numLeds;
    uint8_t* m_dmxBuffer;
    constexpr uint8_t k_numPxls = 3U;
    
public:
    Fixture(std::string name, std::string type, uint16_t numLeds);

    uint8_t getID() const;
    void setUniverse(uint8_t newUniverse) override;
    virtual void update() = 0;
};
