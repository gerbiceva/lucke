#include "Fixture.h"

uint8_t Fixture::s_ID = 0U;

Fixture::Fixture(std::string name, std::string type, uint16_t numLeds)
    : FixtureConfig(name), m_type(type) m_numLeds(numLeds)
{
    m_ID = s_ID++;
    // handle universe settings
}

uint8_t Fixture::getID() const
{
    return m_ID;
}

void Fixture::setUniverse(uint8_t newUniverse) 
{
    m_dmxBuffer = Handler::Sacn::buffer(newUniverse);
}