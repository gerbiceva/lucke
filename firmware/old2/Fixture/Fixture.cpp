#include "Fixture.h"
#include "Handlers/FixtureHandler.h"

Fixture::Fixture(std::string name, std::string type, uint16_t numLeds)
    : FixtureConfig(name), m_type(type), m_numLeds(numLeds)
{
    // m_ledBuffer = new uint8_t[getSiz]

    // Handler::Fixtures::addFixture(this);
    // m_dmxBuffer = Handler::Sacn::buffer(universe);
    
    // handle universe settings
}

uint8_t Fixture::getID() const
{
    return m_ID;
}

void Fixture::setUniverse(uint8_t newUniverse) 
{
    Handler::Sacn::decrementActive(universe);
    universe = newUniverse;
    m_dmxBuffer = Handler::Sacn::buffer(newUniverse);
    m_storage.putUChar("universe", newUniverse);
}

uint16_t Fixture::getLedBufferSize() const
{
    return m_numLeds * k_numPxls;
}


JsonDocument Fixture::toJson() const
{
    JsonDocument doc;
    doc["id"] = m_ID;
    doc["name"] = name;
    doc["universe"] = universe;
    doc["address"] = address;
    doc["preset_index"] = selectedPreset;

    doc["presets"] = presetsToJson();
	// JsonArray jsonArray = doc["presets"].to<JsonArray>();
	// for (int i = 0; i < 15; i++) {
	// 	jsonArray.add();
	// }

    return doc;
}
