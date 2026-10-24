#include "PinHandler.h"

namespace Handler
{
    std::set<uint8_t> Pin::m_usedPins;
    
    bool Pin::available(uint8_t pin)
    {
        if(m_usedPins.find(pin) == m_usedPins.end())
        {
            m_usedPins.emplace(pin);
            return true;
        }
        else 
        {
            return false;
        }
    }
    
    void Pin::toJson(JsonObject& doc)
    {
        JsonArray arr = doc["used_pins"].as<JsonArray>();
    
        for(uint8_t pin : m_usedPins)
        {
            arr.add(pin);
        }
    }
}
