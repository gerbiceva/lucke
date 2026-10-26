#include "PinManager.h"

namespace Handler
{
    std::set<uint8_t> Pin::m_usedPins;
    
    bool Pin::claim(uint8_t pin)
    {
        return m_usedPins.insert(pin).second;
    }
}
