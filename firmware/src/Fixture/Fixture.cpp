#include "Fixture.h"
#include "Handlers/InputHandler.h"
#include <ArduinoJson.h>


Fixture::Fixture(std::string name, std::string type, std::string presets, Traits::InputInterface::InputType input_type)
    : m_inType(input_type),
    Core::Fixture::FixtureConfig(name, type)
{
    deserializeJson(jsonPreset, presets);

    setUniverse(universe);
    setAddress(address);
    setName(name);
    setPreset(selectedPreset);
}

void Fixture::setUniverse(uint8_t new_universe)
{
    universe = new_universe;
    m_storage.putUShort("universe", universe);

    m_srcBuffer = Handler::InputHandler::interface(universe, m_inType)->getBuffer();
    
    updatePresets();
}

void Fixture::setAddress(uint16_t new_address)
{
    address = new_address;
    m_storage.putUShort("address", address);
    
    updatePresets();
}

void Fixture::setPreset(uint8_t new_preset)
{
    selectedPreset = new_preset;
    m_storage.putUChar("preset_index", selectedPreset);

    uint8_t counter = 0;
    for(Traits::OutputInterface* o : m_outputs)
    {
        o->setPreset(jsonPreset["groups"][selectedPreset][counter++]);
    }

    updatePresets();

}

void Fixture::setName(const std::string& other) 
{
    name = other;
    m_storage.putString("name", name);
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
    // InputHandler::::Handler::update();

    for(Traits::OutputInterface* o : m_outputs)
    {
        o->update();
    }
}

// JsonDocument Fixture::selfReportJson() const
// {
//     JsonDocument doc;
//     doc["name"] = name;
//     doc["type"] = type;
//     doc["universe"] = universe;
//     doc["address"] = address;
//     doc["preset"] = selectedPreset;
//     return doc;
// }

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

    // for(Traits::OutputInterface* o : outputs)
    // {
    //     outputs.add(o->describe());
    // }

    return doc;
}
