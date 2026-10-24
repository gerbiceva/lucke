#pragma once
#include <cstdint>
#include <string>

#include "Interfaces/Serializable.h"


namespace Config
{

struct Fixture : public Interface::Serializable
{
    uint8_t m_ID = 0;

    // sACN universe, 1..63999. FixtureHandler gives each new fixture its own.
    uint16_t universe = 1U;
    uint16_t address = 0U;
    uint8_t selectedPreset = 0U;
    uint8_t numPresets = 0U;
    // LEDs on a strip fixture; 0 for fixtures without one. Read at boot,
    // before the fixture is built (see FixtureHandler::addFixture).
    uint16_t numLeds = 0U;

    std::string name = "name";
    std::string type = "type";

    bool highlighted = false;

    void toJson(JsonObject& obj) override;
    JsonDocument toJsonDoc() override;
};

}