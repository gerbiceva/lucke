#pragma once
#include <cstdint>
#include <set>

class PinHandler
{
    static std::set<uint8_t> m_usedPins;
public:
    static bool available(uint8_t pin)
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

};