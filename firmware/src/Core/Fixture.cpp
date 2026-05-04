#include "Fixture.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"
#include "Utils/JsonUtils.h"
#include "Handlers/InputHandler.h"


Fixture::Fixture()    
{
    m_config.m_ID = Config::Fixture::s_ID++;
    obtainSrcBuffer();
}

Fixture::Fixture(Config::Fixture config, std::string presets)
{
    m_config = config;
    m_config.m_ID = Config::Fixture::s_ID++;
    deserializeJson(jsonPreset, presets);
}


Fixture::Fixture(std::string name, std::string type, std::string presets)
{
    m_config.m_ID = Config::Fixture::s_ID++;
    m_config.name = name;
    m_config.type = type;

    deserializeJson(jsonPreset, presets);
    // obtainSrcBuffer();
}

void Fixture::obtainSrcBuffer()
{
    m_dmxIn = Handler::InputHandler::instance().newInterface(m_config.universe);
    m_srcBuffer = m_dmxIn->getBuffer();
}

void Fixture::configureOutput(Traits::OutputInterface* output)
{
    m_outputs.push_back(output);
    m_outputs.back()->bind();

    m_outputs.back()->setSrcBuffer(m_srcBuffer + m_config.address + m_lastOffset);
    m_lastOffset += m_outputs.back()->getSize();
    updatePresets();
}

uint8_t* Fixture::getSrcBuffer() 
{ 
    return m_srcBuffer; 
}

uint8_t* Fixture::getOffsetSrcBuffer() 
{ 
    return m_srcBuffer + m_config.address; 
}

uint8_t Fixture::id() const 
{ 
    return m_config.m_ID; 
}

const std::string& Fixture::getName() const 
{ 
    return m_config.name; 
}

const std::string& Fixture::getType() const 
{ 
    return m_config.type; 
}

bool& Fixture::getHighlighted()
{
    return m_config.highlighted;
}


void Fixture::getPresets(JsonObject& obj)
{
    obj[this->m_config.name] = jsonPreset;
}

void Fixture::setUniverse(uint8_t new_universe)
{
    m_dmxIn = Handler::InputHandler::instance().interface(new_universe, m_config.universe);
    m_srcBuffer = m_dmxIn->getBuffer();
    m_config.universe = new_universe;
    // updatePresets();
}

void Fixture::setAddress(uint16_t new_address)
{
    m_config.address = new_address;
    // updatePresets();
}

void Fixture::setPreset(uint8_t new_preset)
{
    m_config.selectedPreset = new_preset;
    // updatePresets();
}

void Fixture::setName(const std::string& other) 
{
    m_config.name = other;
}

void Fixture::setType(const std::string& other) 
{
    m_config.type = other;
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
        o->postupdate();
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
    obtainSrcBuffer();
    // setUniverse(m_config.universe);
    setAddress(m_config.address);
    setPreset(m_config.selectedPreset);
    updatePresets();
}

JsonDocument Fixture::toJsonDoc()
{
    JsonDocument doc;
    doc["id"] = m_config.m_ID;
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
    m_config.toJson(doc);
    doc["footprint"] = m_lastOffset;
}

void Fixture::toJsonFull(JsonObject& doc)
{
    m_config.toJson(doc);
    doc["footprint"] = m_lastOffset;
    doc["preset"] = getSelectedPresetName();

    JsonObject inputDoc = doc["input"].to<JsonObject>();
    m_dmxIn->toJson(inputDoc);
    JsonArray objectsArray = doc["objects"].to<JsonArray>();

    for(Traits::OutputInterface* o : m_outputs)
    {
        JsonObject entry = objectsArray.add<JsonObject>();
        o->toJson(entry);
    }

    // doc["presets"] = jsonPreset;
}
