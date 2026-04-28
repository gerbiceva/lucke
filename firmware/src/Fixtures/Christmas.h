#pragma once
#include "Core/Fixture.h"
#include "Output/HardwareLED1D.h"

template<uint8_t TPin>
struct Christmas : public Fixture
{
    static constexpr uint16_t N = 80U;
    static constexpr uint16_t BYTES = N * 3;

    Christmas()
		: Fixture("neki", "Christmas",  R"(
    {
        "groups": [
        {
            "name": "group by 1",
            "settings": [
                { "num_groups": 80 }
            ]
        },
        {
            "name": "group by 2",
            "settings": [
                { "num_groups": 40 }
            ]
        },
        {
            "name": "group by 4",
            "settings": [
                { "num_groups": 20 }
            ]
        },
        {
            "name": "group by 8",
            "settings": [
                { "num_groups": 10 }            
            ]
        }
        ]
    }
    
    )")
    {
        addOutput<Output::HardwareLED1D<WS2812B, TPin, GRB>> (N);
    }

    void wifiAnimation() override
    {
        static uint16_t off = 0;
        getSrcBuffer()[off] = 255;
        getSrcBuffer()[off == 0 ? (BYTES - 1) : off - 1] = 0;
        off = (off + 1) % BYTES;
        vTaskDelay(20);
    }

    void highlight() override
    {
        memset(getOffsetSrcBuffer(), 255, BYTES);
    }
};