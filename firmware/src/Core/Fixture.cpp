#include "Fixture.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"
#include "Utils/JsonUtils.h"
#include "Handlers/InputHandler.h"


Fixture::Fixture()    
{
    m_config.obtainID();
}

Fixture::Fixture(Config::Fixture config, std::string presets)
{
    m_config = config;
    m_config.obtainID();
    deserializeJson(jsonPreset, presets);
    countPresets();
}


Fixture::Fixture(std::string name, std::string type, std::string presets)
{
    m_config.obtainID();
    m_config.name = name;
    m_config.type = type;

    deserializeJson(jsonPreset, presets);
    countPresets();
}

void Fixture::countPresets()
{
    JsonArray arr1 = jsonPreset["groups"].as<JsonArray>();
    m_config.numPresets = arr1.size();
}

void Fixture::configureOutput(Traits::OutputInterface* output)
{
    m_outputs.push_back(output);
    m_outputs.back()->bind();

    m_outputs.back()->setSrcBuffer(m_srcBuffer + m_config.address + m_lastOffset);
    m_lastOffset += m_outputs.back()->getSize();
    updatePresets();

    m_outputs.back()->appendBasicInfo(outputInfoJson);
    serializeJson(outputInfoJson, outputInfoSerialized);
}

uint8_t* Fixture::getSrcBuffer() 
{ 
    return m_srcBuffer; 
}

uint8_t* Fixture::getOffsetSrcBuffer() 
{ 
    return m_srcBuffer + m_config.address; 
}



void Fixture::setUniverse(uint8_t new_universe, bool first)
{
    if(first)
    {
        this->m_dmxIn = Handler::InputHandler::instance().newInterface(new_universe);
        Utils::Logger::println("HERE");
    }
    else if(new_universe != m_config.universe)
    {
        this->m_dmxIn = Handler::InputHandler::instance().interface(new_universe, m_config.universe);
    }

    m_srcBuffer = m_dmxIn->getBuffer();
    m_config.universe = new_universe;

    uint16_t offset = 0;
    for(Traits::OutputInterface* o : this->m_outputs)
    {
        o->setSrcBuffer(this->m_srcBuffer + m_config.address + offset);
        // Utils::Logger::printf("%d %p\n", id(), m_srcBuffer);
        offset += o->getSize();
    }
}

void Fixture::setAddress(uint16_t new_address)
{
    m_config.address = new_address;
}

void Fixture::setPreset(uint8_t new_preset)
{
    m_config.selectedPreset = new_preset;
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
                    Traits::OutputInterface* o = this->m_outputs[counter++];
                    o->setPreset(d);
                    o->setSrcBuffer(this->m_srcBuffer + m_config.address + offset);
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
    for(Traits::OutputInterface* o : this->m_outputs)
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

void Fixture::fromConfig()
{
    setName(m_config.name);
    setAddress(m_config.address);
    setPreset(m_config.selectedPreset);
    setUniverse(m_config.universe, true);
    updatePresets();
}

void Fixture::fromJson(std::string json) 
{
    JsonDocument doc;
    deserializeJson(doc, json);

    const char* name = Utils::Json::getElement(doc, "name", m_config.name.c_str());
    uint8_t uni = Utils::Json::getElement(doc, "universe", m_config.universe);


    // Utils::Json::updateElement<uint8_t>(doc, "universe", m_config.universe);
    Utils::Json::updateElement<uint16_t>(doc, "address", m_config.address);
    Utils::Json::updateElement<uint8_t>(doc, "presetIndex", m_config.selectedPreset);
    
    setName(name);
    setAddress(m_config.address);
    setPreset(m_config.selectedPreset);
    setUniverse(uni, true);
    Utils::Json::updateElement<uint8_t>(doc, "universe", m_config.universe);

    
    // obtainSrcBuffer();
    // setUniverse(m_config.universe);
    updatePresets();
}

const std::string& Fixture::getBasicInfo()
{
    return outputInfoSerialized;
}

JsonDocument Fixture::toJsonDoc()
{
    JsonDocument doc = m_config.toJsonDoc();
    doc["footprint"] = m_lastOffset;
    return doc;
}

JsonDocument Fixture::configJson()
{
    JsonDocument doc = m_config.toJsonDoc();
    doc["footprint"] = m_lastOffset;
    doc["basic"] = outputInfoJson;
    return doc;
    // JsonObject obj = doc["basic"].to<JsonObject>();
}

void Fixture::toJson(JsonObject& doc)
{
    m_config.toJson(doc);
    doc["footprint"] = m_lastOffset;
}

void Fixture::toJsonFull(JsonObject& doc)
{
    JsonObject conf = doc["config"].to<JsonObject>();
    m_config.toJson(conf);

    doc["footprint"] = m_lastOffset;
    doc["preset"] = getSelectedPresetName();
    doc["input_type"] = m_dmxIn->inputTypeStr();

    JsonArray objectsArray = doc["outputs"].to<JsonArray>();
    for(Traits::OutputInterface* o : m_outputs)
    {
        JsonObject entry = objectsArray.add<JsonObject>();
        o->toJson(entry);
    }
}

uint8_t Fixture::id() const 
{ 
    return m_config.m_ID; 
}

uint8_t Fixture::getUniverse() const
{
    return m_config.universe;
}

uint16_t Fixture::getAddress() const
{
    return m_config.address;
}

uint8_t Fixture::getSelectedPreset() const
{
    return m_config.selectedPreset;
}

uint8_t Fixture::getNumPresets() const
{
    return m_config.numPresets;
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
    JsonDocument temp;
    temp["selected"] = m_config.selectedPreset;
    temp["presets"] = jsonPreset;
    obj[this->m_config.name] = temp;
}