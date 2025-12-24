#include "Fixture.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"
#include "Engine.h"
#include "Utils/JsonUtils.h"


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

uint8_t* Fixture::getSrcBuffer() 
{ 
    return m_srcBuffer; 
}

uint8_t Fixture::id() const 
{ 
    return m_ID; 
}

const std::string& Fixture::getName() const 
{ 
    return m_config.name; 
}

void Fixture::setUniverse(uint8_t new_universe)
{
    m_config.universe = new_universe;
    m_dmxIn = Engine::instance().getDMXInput(m_config.universe);
    m_srcBuffer = m_dmxIn->getBuffer();
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
    bool valid = false;

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
            valid = true;
            break;
        }
    }

    if(!valid)
    {

        Utils::Logger::printf("[FIXTURE] Invalid preset index: %d\n", m_config.selectedPreset);
    }
}

void Fixture::update()
{
    for(Traits::OutputInterface* o : m_outputs)
    {
        o->update();
    }
}

std::string Fixture::getSelectedPresetName()
{
    JsonArray arr1 = jsonPreset["groups"].as<JsonArray>();
    uint8_t c = 0;

    for(JsonDocument o : arr1)
    {
        if(c++ == m_config.selectedPreset)
        {
            const char* name;
            name = o["name"];
            return name;
        }
    }

    return "Invalid name";
}

void Fixture::fromJson(std::string json) 
{
    JsonDocument doc;
    deserializeJson(doc, json);

    const char* name = Utils::Json::getElement(doc, "name", m_config.name.c_str());
    
    Utils::Json::updateElement<uint8_t>(doc, "universe", m_config.universe);
    Utils::Json::updateElement<uint16_t>(doc, "address", m_config.address);
    Utils::Json::updateElement<uint8_t>(doc, "presetIndex", m_config.selectedPreset);
    
    setName(name);
    setUniverse(m_config.universe);
    setAddress(m_config.address);
    setPreset(m_config.selectedPreset);
}

JsonDocument Fixture::toJsonDoc()
{
    JsonDocument doc;
    doc["id"] = m_ID;
    doc["name"] = m_config.name;
    doc["type"] = m_config.type;
    doc["footprint"] = m_lastOffset;
    doc["universe"] = m_config.universe;
    doc["address"] = m_config.address;
    doc["presetIndex"] = m_config.selectedPreset;
    return doc;
}


void Fixture::toJson(JsonObject& doc)
{
    doc["id"] = m_ID;
    doc["name"] = m_config.name;
    doc["type"] = m_config.type;
    doc["footprint"] = m_lastOffset;
    doc["universe"] = m_config.universe;
    doc["address"] = m_config.address;
    doc["presetIndex"] = m_config.selectedPreset;
    //doc["preset"] = getSelectedPresetName();
    //doc["input"] = m_dmxIn->toJson();
}

void Fixture::toJsonFull(JsonObject& doc)
{
    doc["id"] = m_ID;
    doc["name"] = m_config.name;
    doc["type"] = m_config.type;
    doc["footprint"] = m_lastOffset;
    doc["universe"] = m_config.universe;
    doc["address"] = m_config.address;
    doc["presetIndex"] = m_config.selectedPreset;
    doc["preset"] = getSelectedPresetName();

    JsonObject inputDoc = doc["input"].to<JsonObject>();
    m_dmxIn->toJson(inputDoc);
    JsonArray objectsArray = doc["objects"].to<JsonArray>();

    for(Traits::OutputInterface* o : m_outputs)
    {
        JsonObject entry = objectsArray.add<JsonObject>();
        o->toJson(entry);
    }

    doc["presets"] = jsonPreset;
}
