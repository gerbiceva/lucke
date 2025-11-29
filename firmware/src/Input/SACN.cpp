#include "SACN.h"

namespace Input 
{
    Sacn::Sacn(uint8_t universe)
        : udp(WiFiUDP()), InputInterface(universe)
    {
        m_type = InputType::SACN;
        recv = new Receiver(udp);
        recv->begin(universe);
    }

    Sacn::~Sacn()
    {
        delete recv;
    }

    // void Sacn::init()
    // {
        
    // }

    void Sacn::update()
    {
        recv->dmx(m_dmxBuffer);
        recv->update();
    }

    JsonDocument Sacn::describe()
    {
        JsonDocument doc;
        doc["id"] = m_ID;
        doc["universe"] = m_universe;
        doc["type"] = "SACN";
        doc["seq_diff"] = recv->seqdiff();
        doc["fps"] = recv->framerate();

        return doc;
    }
}