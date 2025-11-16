#pragma once
#include <vector>
#include "FixtureConfig.h"
#include "Traits/Outputs.h"
#include "Traits/Inputs.h"

#include <ArduinoJson.h>

class Fixture : public Core::Fixture::FixtureConfig
{
    std::vector<Traits::OutputInterface*> m_outputs;
    uint8_t* m_srcBuffer;
    uint16_t m_lastOffset = 0U;

    Traits::InputInterface::InputType m_inType;
    JsonDocument jsonPreset;

    void updatePresets();
public:

    Fixture(std::string name, std::string type, std::string presets, Traits::InputInterface::InputType input_type = Traits::InputInterface::InputType::SACN);

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
    // uint16_t getSrcBufferSize() const { return m_outputs.back()->}

    void setUniverse(uint8_t new_universe);
    void setAddress(uint16_t new_address);
    void setPreset(uint8_t new_preset);
    void setName(const std::string& other);

    virtual void update();
    virtual void wifiAnimation()
    {
        // static uint16_t off = 0;
        // getSrcBuffer()[off] = 255;
        // getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
        // off = (off + 1) % 180;
    }

    // JsonDocument toJson()
    // {
    //     JsonDocument doc;
    //     doc["name"] = name;
    //     doc["type"] = type;
    //     doc["universe"] = universe;
    //     doc["address"] = address;
    //     doc["preset"] = preset;
    //     JsonArray arr = doc["outputs"].as<JsonArray>();
        
        
    //     for(OutputInterface* o : m_outputs)
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