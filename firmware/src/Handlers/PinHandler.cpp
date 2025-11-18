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