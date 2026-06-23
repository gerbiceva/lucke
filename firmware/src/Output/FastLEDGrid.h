#pragma once
#include <FastLED.h>
#include "Base/LEDGridBase.h"
#include "Base/Drivers.h"
#include "Handlers/PinHandler.h"

namespace Output
{
    // FastLED-backed grid. Pins are compile-time (FastLED requirement), so
    // addStrip<TPin>() is a template. All mapping logic lives in LEDGridBase.
    template <template <uint8_t, EOrder> typename TLedType, EOrder TOrder>
    class FastLEDGrid : public LEDGridBase
    {
    public:
        FastLEDGrid(uint16_t w, uint16_t h)
            : LEDGridBase(w, h, FastLEDDriver<TLedType, 0, TOrder>::kind)
        {
        }

        template <uint8_t TPin>
        void addStrip(uint16_t numLeds, GRange xr, GRange yr,
                      uint16_t cellW = 1, uint16_t cellH = 1, bool horizontal = true)
        {
            if (!Handler::Pin::available(TPin))
            {
                Utils::Logger::printf("[GRID] pin %d already used\n", TPin);
                return;
            }

            uint8_t *buf = new uint8_t[numLeds * 3]();
            FastLED.addLeds<TLedType, TPin, TOrder>((CRGB *)buf, numLeds);
            registerStrip(buf, xr, yr, cellW, cellH, horizontal);
        }

        void postupdate() override { FastLED.show(); } // flushes all pins
    };
}
