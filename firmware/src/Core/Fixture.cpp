#include "Fixture.h"
#include "Utils/JsonUtils.h"
#include "Utils/Logger.h"
#include <algorithm>
#include <span>

namespace
{
    constexpr Utils::Log s_log{"FIXTURE"};
}

Fixture::Fixture() 
{
}

Fixture::Fixture(std::string name, std::string type, std::string presets)
{
    m_config.name = std::move(name);
    m_config.type = std::move(type);

    deserializeJson(m_jsonPresets, presets);
    countPresets();
}

void Fixture::update()
{
    std::scoped_lock lock(m_mutex);
    if (m_syncBuffers.empty()) return;

    size_t footprint = 0;
    for (auto& o : m_outputs)
    {
        footprint += o->footprint();
    }

    // Guards against a mis-set address reading past the end of m_buffer.
    if (m_config.address + footprint > m_buffer.size()) return;

    std::span<uint8_t> all{m_buffer};
    std::span<uint8_t> fixture = all.subspan(m_config.address, footprint);

    if (m_config.highlighted)
    {
        std::ranges::fill(fixture, 255);
    }
    else
    {
        for (size_t i = 0; i < m_syncBuffers.size(); i++)
        {
            m_syncBuffers[i]->read(all.subspan(i * Dmx::c_universeSize).first<Dmx::c_universeSize>());
        }
    }

    std::span<const uint8_t> src = fixture;
    for (auto& o : m_outputs)
    {
        o->update(src.first(o->footprint()));
        src = src.subspan(o->footprint());
    }
}

std::unique_lock<std::recursive_mutex> Fixture::lock()
{
    return std::unique_lock(m_mutex);
}

void Fixture::setSyncBuffers(std::vector<std::shared_ptr<SyncBuffer>> bufs)
{
    std::scoped_lock lock(m_mutex);
    m_syncBuffers = std::move(bufs);
    m_buffer.resize(m_syncBuffers.size() * Dmx::c_universeSize, 0);
}

void Fixture::setUniverse(uint16_t new_universe)
{
    std::scoped_lock lock(m_mutex);
    m_config.universe = new_universe;
}

std::vector<std::unique_ptr<Interface::Output>>& Fixture::getOutputs()
{
    return m_outputs;
}

void Fixture::fromConfig()
{
    setPreset(m_config.selectedPreset);
}

void Fixture::fromJson(const std::string& json)
{
    JsonDocument doc;
    deserializeJson(doc, json);

    const char* name = Utils::Json::getElement(doc, "name", m_config.name.c_str());

    Utils::Json::updateElement<uint16_t>(doc, "universe", m_config.universe);
    Utils::Json::updateElement<uint16_t>(doc, "address", m_config.address);
    Utils::Json::updateElement<uint8_t>(doc, "presetIndex", m_config.selectedPreset);

    setName(name);
    setPreset(m_config.selectedPreset);
}

void Fixture::toJson(JsonObject& doc)
{
    info().toJson(doc);
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
    std::scoped_lock lock(m_mutex);
    JsonDocument doc = m_config.toJsonDoc();
    doc["footprint"] = m_lastOffset;
    JsonArray outputs = doc["outputs"].to<JsonArray>();
    for (auto& o : m_outputs)
    {
        JsonObject entry = outputs.add<JsonObject>();
        o->toJson(entry);
    }
    return doc;
}

JsonDocument Fixture::outputsJson()
{
    std::scoped_lock lock(m_mutex);
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
    std::scoped_lock lock(m_mutex);
    JsonDocument doc;
    doc["selected"] = m_config.selectedPreset;
    doc["presets"] = m_jsonPresets;
    return doc;
}

void Fixture::setAddress(uint16_t new_address)
{
    std::scoped_lock lock(m_mutex);
    m_config.address = new_address;
}

void Fixture::setPreset(uint8_t new_preset)
{
    std::scoped_lock lock(m_mutex);

    // Iterate the preset tree by reference (JsonObjectConst/JsonArrayConst) —
    // binding elements to a JsonDocument would deep-copy each group.
    JsonArrayConst groups = m_jsonPresets["groups"].as<JsonArrayConst>();
    if (new_preset >= groups.size())
    {
        s_log.warn("invalid preset index: %u", new_preset);
        return;
    }

    JsonArrayConst settings = groups[new_preset]["settings"].as<JsonArrayConst>();
    if (settings.size() != m_outputs.size())
    {
        s_log.warn("preset %u has %zu settings but fixture has %zu outputs — ignored",
                   new_preset, settings.size(), m_outputs.size());
        return;
    }

    // Only reset once the preset is known to be valid: an invalid one keeps
    // the current preset and footprint instead of leaving footprint 0.
    m_config.selectedPreset = new_preset;
    m_lastOffset = 0;
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
    std::scoped_lock lock(m_mutex);
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

uint16_t Fixture::getUniverse() const
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

std::string Fixture::getName() const 
{ 
    std::scoped_lock lock(m_mutex);
    return m_config.name; 
}

const std::string& Fixture::getType() const 
{ 
    return m_config.type; 
}

void Fixture::setLedCount(uint16_t numLeds)
{
    std::scoped_lock lock(m_mutex);
    m_config.numLeds = numLeds;
}

uint16_t Fixture::ledCount() const
{
    std::scoped_lock lock(m_mutex);
    return m_config.numLeds;
}

void Fixture::toggleHighlight()
{
    std::scoped_lock lock(m_mutex);
    m_config.highlighted = !m_config.highlighted;
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