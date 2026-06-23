#pragma once
#include "LED1D.h"

namespace Output
{
    // FastLED-backed 1D strip. Kept for existing call sites; just pins the
    // FastLED driver with a compile-time pin and forwards it as the runtime
    // pin used for the availability check / reporting.
    template <template <uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class FastLED1D : public LED1D<FastLEDDriver<TLedType, TPin, TOrder>>
    {
    public:
        FastLED1D(uint16_t num_leds)
            : LED1D<FastLEDDriver<TLedType, TPin, TOrder>>(num_leds, TPin)
        {
        }
    };
}
