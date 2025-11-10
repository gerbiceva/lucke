#include "Fixture.h"

void Fixture::setUniverse(uint8_t new_universe)
{
    // setsrc buffer
    m_srcBuffer = Input::Handler::interface(universe, m_inType)->getBuffer();
    // m_srcBuffer = new uint8_t[512]();
    universe = new_universe;
    m_storage.putUShort("universe", universe);
    
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
    for(Output::Outputs* o : m_outputs)
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
    for(Output::Outputs* o : m_outputs)
    {
        o->setSrcBuffer(m_srcBuffer + address + offset);
        offset += o->getSize();
        // o->setPreset(jsonPreset[selectedPreset][counter++]);
    }
}

void Fixture::update()
{
    // Input::Handler::update();

    for(Output::Outputs* o : m_outputs)
    {
        o->update();
    }
}