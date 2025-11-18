#pragma once
#include <cstdint>
#include <set>

class PinHandler
{
    static std::set<uint8_t> m_usedPins;
public:
    static bool available(uint8_t pin);

};