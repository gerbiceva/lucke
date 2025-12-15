#include "Fixture.h"
// #include "Handlers/InputHandler.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"
#include "Engine.h"


uint8_t Fixture::s_ID = 0;

Fixture::Fixture()    
{
    m_ID = s_ID++;    
}

Fixture::Fixture(std::string name, std::string type, std::string presets)
{
    m_ID = s_ID++;
    m_config.name = name;
    m_config.type = type;

    deserializeJson(jsonPreset, presets);
}

void Fixture::setUniverse(uint8_t new_universe)
{
    m_config.universe = new_universe;
    m_srcBuffer = Engine::instance().getDMXInput(m_config.universe)->getBuffer();    
    updatePresets();
}

void Fixture::setAddress(uint16_t new_address)
{
    m_config.address = new_address;
    updatePresets();
}

void Fixture::setPreset(uint8_t new_preset)
{
    m_config.selectedPreset = new_preset;
    updatePresets();
}

void Fixture::setName(const std::string& other) 
{
    m_config.name = other;
}

void Fixture::updatePresets()
{
    JsonArray arr1 = jsonPreset["groups"].as<JsonArray>();
    uint8_t c = 0;

    for(JsonDocument o : arr1)
    {
        if(c++ == m_config.selectedPreset)
        {
            JsonArray presets = o["settings"].as<JsonArray>();
            uint16_t offset = 0;
            uint8_t counter = 0;
            for(JsonDocument d : presets)
            {
                if(m_outputs.size() == presets.size())
                {
                    Traits::OutputInterface* o = m_outputs[counter++];
                    o->setPreset(d);
                    o->setSrcBuffer(m_srcBuffer + m_config.address + offset);
                    offset += o->getSize();
                }
            }
            break;
        }
    }

}

void Fixture::update()
{
    for(Traits::OutputInterface* o : m_outputs)
    {
        o->update();
    }
}

void Fixture::fromJson(std::string json) 
{
    JsonDocument doc;
    deserializeJson(doc, json);

    const char* name = doc["name"];
    setName(name);
    setUniverse(doc["universe"]);
    setAddress(doc["address"]);
    setPreset(doc["presetIndex"]);
}


JsonDocument Fixture::toJson()
{
    JsonDocument doc;
    doc["id"] = m_ID;
    doc["name"] = m_config.name;
    doc["type"] = m_config.type;
    doc["universe"] = m_config.universe;
    doc["address"] = m_config.address;
    doc["presetIndex"] = m_config.selectedPreset;

    // doc["outputs"] = JsonDocument();
	// JsonArray outputs = doc["outputs"].to<JsonArray>();

    // for(Traits::OutputInterface* o : m_outputs)
    // {
    //     outputs.add(o->toJson());
    // }

    // doc["presets"] = jsonPreset;

    return doc;
}

JsonDocument Fixture::toJsonFull()
{
    JsonDocument doc;
    doc["id"] = m_ID;
    doc["name"] = m_config.name;
    doc["type"] = m_config.type;
    doc["universe"] = m_config.universe;
    doc["address"] = m_config.address;
    doc["presetIndex"] = m_config.selectedPreset;

    doc["outputs"] = JsonDocument();
	JsonArray outputs = doc["outputs"].to<JsonArray>();

    for(Traits::OutputInterface* o : m_outputs)
    {
        outputs.add(o->toJson());
    }

    doc["presets"] = jsonPreset;

    return doc;
}
