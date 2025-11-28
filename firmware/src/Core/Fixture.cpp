#include "Fixture.h"
#include "Handlers/InputHandler.h"
#include <ArduinoJson.h>
#include "Utils/Logger.h"


Fixture::Fixture()
    : m_inType(Traits::InputInterface::InputType::DMX)
    
{

}

Fixture::Fixture(std::string name, std::string type, std::string presets, Traits::InputInterface::InputType input_type)
    : m_inType(input_type),
    Core::Fixture::FixtureConfig(name, type)
{
    deserializeJson(jsonPreset, presets);

    setUniverse(1);
    setAddress(this->address);
    setName(this->name);
    // setPreset(selectedPreset);
}

void Fixture::setUniverse(uint8_t new_universe)
{
    this->universe = new_universe;
    m_storage.putUShort("universe", universe);

    m_srcBuffer = Handler::InputHandler::interface(new_universe, m_inType)->getBuffer();
    
    updatePresets();
}

void Fixture::setAddress(uint16_t new_address)
{
    this->address = new_address;
    m_storage.putUShort("address", this->address);
    
    updatePresets();
}

void Fixture::setPreset(uint8_t new_preset)
{
    this->selectedPreset = new_preset;
    m_storage.putUChar("preset_index", this->selectedPreset);

    uint8_t counter = 0;
    for(Traits::OutputInterface* o : m_outputs)
    {
        o->setPreset(jsonPreset["groups"][this->selectedPreset]["settings"][counter++]);
    }

    updatePresets();

}

void Fixture::setName(const std::string& other) 
{
    this->name = other;
    m_storage.putString("name", this->name);
}

void Fixture::updatePresets()
{
    uint16_t offset = 0;
    // uint8_t counter = 0;
    for(Traits::OutputInterface* o : m_outputs)
    {
        o->setSrcBuffer(m_srcBuffer + address + offset);
        offset += o->getSize();
        // o->setPreset(jsonPreset[selectedPreset][counter++]);
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
    doc["name"] = name;
    doc["type"] = type;
    doc["universe"] = universe;
    doc["address"] = address;
    doc["preset"] = selectedPreset;

    doc["outputs"] = JsonDocument();
	JsonArray outputs = doc["outputs"].to<JsonArray>();

    for(Traits::OutputInterface* o : m_outputs)
    {
        outputs.add(o->describe());
    }

    return doc;
}
