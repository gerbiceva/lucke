#include "SacnSource.h"

namespace Input 
{
    SacnSource::~SacnSource()
    {
        m_sacn.stop();
    }

    void SacnSource::begin(uint8_t universe)
    {
        m_universe = universe;
        m_sacn.begin(m_universe);
    }

    void SacnSource::stop() 
    {
        m_sacn.stop();
    }

    bool SacnSource::poll(uint8_t* buff)
    {
        return m_sacn.dmx(buff);
    }

    void SacnSource::toJson(JsonObject& obj)
    {
        obj["type"] = "SACN";
        obj["universe"] = m_universe;
    }
}