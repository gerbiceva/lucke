#include "PinManager.h"

namespace Handler
{
    std::set<uint8_t> Pin::m_usedPins;
    
    bool Pin::claim(uint8_t pin)
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

}
