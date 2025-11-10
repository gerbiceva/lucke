#pragma once
#include <vector>
#include "FixtureConfig.h"
#include "Output/Outputs.h"
#include "Input/InputHandler.h"
#include <ArduinoJson.h>

// using Preset = std::pair<std::string, std::vector<std::vector<uint16_t>>>;
// struct Presets
// {
//     std::vector<Presets> presets;
// };

class Fixture : public Core::Fixture::FixtureConfig
{
    std::vector<Output::Outputs*> m_outputs;
    uint8_t* m_srcBuffer;
    uint16_t m_lastOffset = 0U;

    Input::InputType m_inType;
    JsonDocument jsonPreset;

    void updatePresets();
public:

// TODO: src buffer is null
    Fixture(std::string name, std::string type, std::string presets, Input::InputType input_type = Input::InputType::SACN)
        : m_inType(input_type),
          Core::Fixture::FixtureConfig(name, type)
    {
        deserializeJson(jsonPreset, presets);

        setUniverse(universe);
        setAddress(address);
        setName(name);
        setPreset(selectedPreset);
    }

    template<typename TOutput, typename... Args>
    void addOutput(Args&&... args)
    {
        m_outputs.push_back(new TOutput(args...));
        m_outputs.back()->bind();

        m_outputs.back()->setSrcBuffer(m_srcBuffer + address + m_lastOffset);
        m_lastOffset += m_outputs.back()->getSize();
        updatePresets();
    }

    uint8_t* getSrcBuffer() { return m_srcBuffer; }

    void setUniverse(uint8_t new_universe);
    void setAddress(uint16_t new_address);
    void setPreset(uint8_t new_preset);
    void setName(const std::string& other);

    virtual void update();

    // JsonDocument toJson()
    // {
    //     JsonDocument doc;
    //     doc["name"] = name;
    //     doc["type"] = type;
    //     doc["universe"] = universe;
    //     doc["address"] = address;
    //     doc["preset"] = preset;
    //     JsonArray arr = doc["outputs"].as<JsonArray>();
        
        
    //     for(Output::Outputs* o : m_outputs)
    //     {
    //         arr.add(o->toJson());
    //     }


    //     return doc;
    // }
};

// class Fixture1D : public Fixture
// {
// public:
//     Fixture1D(std::string name, std::string type)
//         : Fixture(name, type)
//     {}

// };