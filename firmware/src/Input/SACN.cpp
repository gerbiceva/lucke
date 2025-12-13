#include "SACN.h"

namespace Input 
{
    Sacn::Sacn(uint8_t universe)
        : InputInterface(universe)
    {
        m_type = InputType::SACN;
        m_sacn.begin(universe);
    }

    Sacn::~Sacn()
    {
        m_sacn.stop();
    }

    void Sacn::update()
    {
        m_sacn.dmx(m_dmxBuffer);
    }

    JsonDocument Sacn::toJson()
    {
        JsonDocument doc;
        doc["id"] = m_ID;
        doc["universe"] = m_universe;
        doc["type"] = "SACN";
        doc["seq_diff"] = m_sacn.getSeqDiff();
        // doc["fps"] = recv->framerate();

        return doc;
    }
}