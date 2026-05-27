#pragma once
#include <cstdint>
#include <string>

#include "Interfaces/Serializable.h"


namespace Config
{

struct Fixture : public Interface::Serializable
{
    uint8_t m_ID = 0;

    uint8_t universe = 8U;
    uint16_t address = 0U;
    uint8_t selectedPreset = 0U;
    uint8_t numPresets = 0U;

    std::string name = "name";
    std::string type = "type";

    bool highlighted = false;

    void toJson(JsonObject& obj) override;
    JsonDocument toJsonDoc() override;
};

}