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
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_syncBuffers.empty()) return;

    if (m_config.highlighted)
    {
        memset(&m_buffer[m_config.address], 255, m_lastOffset);
    }
    else
    {
        for (size_t i = 0; i < m_syncBuffers.size(); i++)
        {
            m_syncBuffers[i]->read(&m_buffer[i * 512]);
        }
    }

    uint8_t* buff = m_buffer.data() + m_config.address;

    for(auto o : m_outputs)
    {
        o->update(buff);
        buff += o->footprint();
    }
}

void Fixture::setSyncBuffers(std::vector<std::shared_ptr<SyncBuffer>> bufs)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_syncBuffers = std::move(bufs);
    m_buffer.resize(m_syncBuffers.size() * 512, 0);
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
    doc["footprint"] = m_lastOffset;
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

JsonDocument Fixture::outputsJson()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    JsonDocument doc;
    JsonArray outputs = doc.to<JsonArray>();

    JsonArrayConst groups = m_jsonPresets["groups"].as<JsonArrayConst>();

    for (size_t j = 0; j < m_outputs.size(); j++)
    {
        JsonObject entry = outputs.add<JsonObject>();
        m_outputs[j]->describe(entry);

        // footprint is per-preset: index i holds the footprint under preset i.
        JsonArray fps = entry["footprint"].to<JsonArray>();
        for (JsonObjectConst g : groups)
        {
            JsonArrayConst settings = g["settings"].as<JsonArrayConst>();
            JsonDocument s;
            if (j < settings.size())
                s.set(settings[j]);
            fps.add(m_outputs[j]->footprintFor(s));
        }
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
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.address = new_address;
}

void Fixture::setPreset(uint8_t new_preset)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.selectedPreset = new_preset;
    m_lastOffset = 0;

    // Iterate the preset tree by reference (JsonObjectConst/JsonArrayConst) —
    // binding elements to a JsonDocument would deep-copy each group.
    JsonArrayConst groups = m_jsonPresets["groups"].as<JsonArrayConst>();
    if (new_preset >= groups.size())
    {
        Utils::Logger::printf("[FIXTURE] Invalid preset index: %d\n", new_preset);
        return;
    }

    JsonArrayConst settings = groups[new_preset]["settings"].as<JsonArrayConst>();
    if (settings.size() != m_outputs.size())
    {
        Utils::Logger::printf(
            "[FIXTURE] Preset %d has %u settings but fixture has %u outputs — ignored\n",
            new_preset, (unsigned)settings.size(), (unsigned)m_outputs.size());
        return;
    }

    for (size_t i = 0; i < m_outputs.size(); i++)
    {
        JsonDocument d;
        d.set(settings[i]);     // interface takes a JsonDocument&; copy just this setting
        m_outputs[i]->setPreset(d);
        m_lastOffset += m_outputs[i]->footprint();
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

uint16_t Fixture::getFootprint() const
{
    return m_lastOffset;
}

uint8_t Fixture::getNumUniverses() const
{
    return m_syncBuffers.size();
}

void Fixture::countPresets()
{
    JsonArray arr1 = m_jsonPresets["groups"].as<JsonArray>();
    m_config.numPresets = arr1.size();
}