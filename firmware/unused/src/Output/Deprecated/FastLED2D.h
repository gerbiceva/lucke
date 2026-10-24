#pragma once
#include "LED2D.h"

namespace Output
{
    // FastLED-backed 2D matrix. Kept for existing call sites.
    template <template <uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class FastLED2D : public LED2D<FastLEDDriver<TLedType, TPin, TOrder>>
    {
    public:
        FastLED2D(uint16_t num_leds)
            : LED2D<FastLEDDriver<TLedType, TPin, TOrder>>(num_leds, TPin)
        {
        }
    };
}
