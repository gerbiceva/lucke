#pragma once
#include <Arduino.h>
#include "Core/Fixture.h"
#include "Output/FastLED2D.h"

namespace Fixtures
{

// Three 8x8 WS2815 panels stacked vertically on pins 5, 6, 7
struct Grid2815_8x24 : public Fixture
{
    static constexpr uint16_t W = 8;
    static constexpr uint16_t H = 8;
    static constexpr uint16_t PANELS = 3;
    static constexpr uint16_t LEDS_PER_PANEL = W * H;
    static constexpr uint16_t BYTES = LEDS_PER_PANEL * 3 * PANELS;

    Grid2815_8x24()
        : Fixture("name", "Grid2815_8x24", R"({
            "groups": [{
                "name": "8x24 vertical",
                "settings": [
                    { "width": 8, "height": 8, "x_offset": 0, "y_offset": 0,  "serpentine": true },
                    { "width": 8, "height": 8, "x_offset": 0, "y_offset": 8,  "serpentine": true },
                    { "width": 8, "height": 8, "x_offset": 0, "y_offset": 16, "serpentine": true }
                ]
            }]
        })")
    {
        addOutput<Output::FastLED2D<WS2815, 5, RGB>>(LEDS_PER_PANEL);
        addOutput<Output::FastLED2D<WS2815, 6, RGB>>(LEDS_PER_PANEL);
        addOutput<Output::FastLED2D<WS2815, 7, RGB>>(LEDS_PER_PANEL);
    }

    void wifiAnimation() override
    {
        m_animBuf[m_animOff] = 255;
        m_animBuf[m_animOff == 0 ? (BYTES - 1) : m_animOff - 1] = 0;
        m_animOff = (m_animOff + 1) % BYTES;

        uint8_t* ptr = m_animBuf;
        for (auto o : getOutputs())
        {
            o->update(ptr);
            ptr += o->footprint();
        }

        vTaskDelay(20);
    }

private:
    uint8_t m_animBuf[BYTES] = {};
    uint16_t m_animOff = 0;
};

}
