#pragma once
#include <cstdint>
#include <FastLED.h>
#include "Hardware/WS2815.h"

namespace Output
{
    // A "driver" is a small policy that binds an RGB byte buffer to a piece of
    // LED hardware and flushes it. The dimension classes (LED1D/LED2D/grids)
    // are written against this interface so they work irrespective of whether
    // the backend is FastLED or the native WS2815 SPI driver.
    //
    // Contract:
    //   void init(uint16_t num_leds, uint8_t* buffer, int pin)  - bind buffer
    //   void show()                                             - flush to LEDs
    //   static constexpr const char* kind                       - report label

    // FastLED backend. Pin/type/order are compile-time (FastLED requirement);
    // the runtime `pin` arg is ignored for binding and only kept for reporting.
    template <template <uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    struct FastLEDDriver
    {
        static constexpr const char *kind = "FastLED";

        void init(uint16_t num_leds, uint8_t *buffer, int /*pin*/)
        {
            FastLED.addLeds<TLedType, TPin, TOrder>((CRGB *)buffer, num_leds)
                .clearLeds();
        }

        void show() { FastLED.show(); }
    };

    // Native WS2815 SPI backend. Pin is selected at runtime.
    struct WS2815Driver
    {
        static constexpr const char *kind = "WS2815";

        Core::WS2815 dev;

        void init(uint16_t num_leds, uint8_t *buffer, int pin)
        {
            dev.init(pin, {buffer, num_leds * 3u});
        }

        void show() { dev.show(); }
    };
}
