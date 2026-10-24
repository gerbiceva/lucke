#pragma once
#include <cstdint>
#include <set>

namespace Handler
{
    class Pin
    {
        static std::set<uint8_t> m_usedPins;
    public:
        // Marks the pin as used. False if something already claimed it.
        static bool claim(uint8_t pin);
    };
}
