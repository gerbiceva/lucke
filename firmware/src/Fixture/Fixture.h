#pragma once
#include "Output/HardwareLED.h"
#include "Handlers/SacnHandler.h"
#include "FixtureConfig.h"
#include "Preset.h"
#include <ArduinoJson.h>


class Fixture : public Output::HardwareLED, public FixtureConfig
{
    std::string m_type;
protected:
	uint16_t m_numLeds;
    uint8_t* m_dmxBuffer;
    const uint8_t k_numPxls = 3U;
    
public:
    Fixture(std::string name, std::string type, uint16_t numLeds);

    uint8_t getID() const;
    void setUniverse(uint8_t newUniverse) override;
    virtual void update() = 0;

    [[nodiscard]] virtual JsonDocument presetsToJson() const = 0;
    [[nodiscard]] JsonDocument toJson() const;
    // virtual std::string getJsonString() const = 0;
};
