#include "PinHandler.h"

std::set<uint8_t> PinHandler::m_usedPins;

bool PinHandler::available(uint8_t pin)
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

JsonDocument PinHandler::describe()
{
    JsonDocument doc;
    doc["used_pins"] = JsonDocument();
    JsonArray arr = doc["used_pins"].to<JsonArray>();

    for(uint8_t pin : m_usedPins)
    {
        arr.add(pin);
    }

    return doc;
}