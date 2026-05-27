#include "Fixture.h"
#include "Utils/JsonUtils.h"

Fixture::Fixture() 
{
}

Fixture::Fixture(std::string name, std::string type, std::string presets)
{
    m_config.name = name;
    m_config.type = type;

    deserializeJson(m_jsonPresets, presets);
    countPresets();
}

void Fixture::_addOutput(Interface::Output* output)
{
    m_outputs.push_back(output);
    m_outputs.back()->bind();

    m_lastOffset += m_outputs.back()->footprint();
}

void Fixture::update()
{
    if (!m_synbuffer) return;

    if (m_config.highlighted)
    {
        memset(&m_buffer[m_config.address], 255, m_lastOffset);
    }
    else
    {
        m_buffer = m_synbuffer->read();
    }

    uint8_t* buff = &m_buffer[0] + m_config.address;

    for(auto o : m_outputs)
    {
        o->update(buff);
        buff += o->footprint();
    }
}

void Fixture::setSyncBuffer(std::shared_ptr<SyncBuffer> buf)
{
    Utils::Logger::println("SET");
    m_synbuffer = buf;
}

void Fixture::setUniverse(uint8_t new_universe)
{
    m_config.universe = new_universe;
}

std::vector<Interface::Output*>& Fixture::getOutputs()
{
    return m_outputs;
}

void Fixture::fromConfig()
{
    setPreset(m_config.selectedPreset);
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
    setPreset(m_config.selectedPreset);
}

void Fixture::toJson(JsonObject& doc)
{
    m_config.toJson(doc);

    // JsonArray outputs = doc["outputs"].to<JsonArray>();
    // for (auto o : m_outputs)
    // {
    //     JsonObject out = outputs.add<JsonObject>();
    //     // o->appendBasicInfo(doc);
    // }
}

JsonDocument Fixture::toJsonDoc()
{
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    toJson(obj);
    return doc;
}

JsonDocument Fixture::configJson()
{
    JsonDocument doc = m_config.toJsonDoc();
    doc["footprint"] = m_lastOffset;
    JsonArray outputs = doc["outputs"].to<JsonArray>();
    for (auto o : m_outputs)
    {
        JsonObject entry = outputs.add<JsonObject>();
        o->toJson(entry);
    }
    return doc;
}

JsonDocument Fixture::presetsJson()
{
    JsonDocument doc;
    doc["selected"] = m_config.selectedPreset;
    doc["presets"] = m_jsonPresets;
    return doc;
}

void Fixture::setAddress(uint16_t new_address)
{
    m_config.address = new_address;
}

void Fixture::setPreset(uint8_t new_preset)
{
    m_config.selectedPreset = new_preset;
    m_lastOffset = 0;

    JsonArray arr1 = m_jsonPresets["groups"].as<JsonArray>();
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
                    auto o = this->m_outputs[counter++];
                    o->setPreset(d);
                    m_lastOffset += o->footprint();
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

void Fixture::setName(const std::string& other) 
{
    m_config.name = other;
}

void Fixture::setId(uint8_t id)
{
    m_config.m_ID = id;
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

void Fixture::countPresets()
{
    JsonArray arr1 = m_jsonPresets["groups"].as<JsonArray>();
    m_config.numPresets = arr1.size();
}