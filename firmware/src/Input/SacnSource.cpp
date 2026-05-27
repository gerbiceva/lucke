#include "SacnSource.h"

namespace Input 
{
    SacnSource::~SacnSource()
    {
        m_sacn.stop();
    }

    // void Sacn::reinit()
    // {
    //     m_sacn.stop();
    //     m_sacn.begin(m_universe);
    // }

    void SacnSource::begin(uint8_t universe) 
    {
        m_universe = universe;
        m_sacn.begin(m_universe);
    }

    void SacnSource::stop() 
    {
        m_sacn.stop();
    }

    void SacnSource::poll(uint8_t* buff)
    {
        m_sacn.dmx(buff);
    }

    void SacnSource::toJson(JsonObject& obj)
    {
        obj["type"] = "SACN";
        obj["universe"] = m_universe;
    }
    // void SacnSource::toJson(JsonObject& doc)
    // {
    //     doc["id"] = m_ID;
    //     doc["type"] = "SACN";
    //     doc["universe"] = m_universe;
    //     // JsonArray arr = doc["seq_diffs"].to<JsonArray>();
        
    //     // for(auto& el : m_sacn.getSeqDiffs())
    //     // {
    //     //     arr.add(el);
    //     // }
    //     JsonArray arr1 = doc["9_bytes"].to<JsonArray>();
    //     for(uint8_t i = 0; i < 9; i++)
    //     {
    //         arr1.add(m_dmxBuffer[i]);
    //     }

    // }

    // JsonDocument Sacn::toJsonFull()
    // {
    //     JsonDocument doc;
    //     doc["id"] = m_ID;
    //     doc["universe"] = m_universe;
    //     doc["type"] = "SACN";
    //     doc["seq_diff"] = m_sacn.getSeqDiff();
    //     // doc["fps"] = recv->framerate();

    //     return doc;
    // }
}