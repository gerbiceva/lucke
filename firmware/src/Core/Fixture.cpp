#include "Fixture.h"
// #include "Handlers/InputHandler.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"
#include "Engine.h"


uint8_t Fixture::s_ID = 0;

Fixture::Fixture()    
{
    Utils::Logger::println("Empty fix");
    m_ID = s_ID++;
    
    init();
}

Fixture::Fixture(std::string name, std::string type, std::string presets)
{
    m_ID = s_ID++;
    m_config.name = name;
    m_config.type = type;

    deserializeJson(jsonPreset, presets);
    init();
}

// Fixture::~Fixture()
// {
//     m_storage->
// }

void Fixture::loadIfExist()
{
    m_storage = new Utils::Storage(std::to_string(m_ID));

    // if(m_storage->isKey("universe"))
    // {
    //     m_config.universe = m_storage->getUChar("universe");
    //     m_config.address = m_storage->getUChar("address");
    //     m_config.selectedPreset = m_storage->getUChar("selectedPreset");
    //     m_config.name = m_storage->getString("name");
    // }
}

void Fixture::init()
{
    loadIfExist();
    m_srcBuffer = Engine::instance().getDMXInput(m_config.universe)->getBuffer();
    updatePresets();
}

void Fixture::setUniverse(uint8_t new_universe)
{
    m_config.universe = new_universe;
    m_storage->putUChar("universe", new_universe);

    m_srcBuffer = Engine::instance().getDMXInput(m_config.universe)->getBuffer();    
    updatePresets();
}

void Fixture::setAddress(uint16_t new_address)
{
    m_config.address = new_address;
    m_storage->putUShort("address", m_config.address);
    
    updatePresets();
}

void Fixture::setPreset(uint8_t new_preset)
{
    m_config.selectedPreset = new_preset;
    m_storage->putUChar("selectedPreset", m_config.selectedPreset);

    updatePresets();
}

void Fixture::setName(const std::string& other) 
{
    m_config.name = other;
    m_storage->putString("name", m_config.name);
}

void Fixture::updatePresets()
{
    uint16_t offset = 0;
    uint8_t counter = 0;
    // std::string test;
    for(Traits::OutputInterface* o : m_outputs)
    {
        o->setSrcBuffer(m_srcBuffer + m_config.address + offset);
        offset += o->getSize();
        // serializeJson(jsonPreset, test);
        // Utils::Logger::println(test.c_str());

        // o->setPreset(jsonPreset["groups"][selectedPreset]["settings"][counter++]);
    }
}

void Fixture::update()
{
    for(Traits::OutputInterface* o : m_outputs)
    {
        o->update();
    }
}

JsonDocument Fixture::describe()
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
        outputs.add(o->describe());
    }

    // doc["presets"] = jsonPreset;

    return doc;
}
