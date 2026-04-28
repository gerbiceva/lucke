#pragma once
#include "Core/Fixture.h"
#include "Output/HardwareLED1D.h"

struct Astera144 : public Fixture
{
    static constexpr uint16_t N = 144U;
    static constexpr uint16_t BYTES = N * 3;

    Astera144()
		: Fixture("name", "Astera144",  R"(
    {
        "groups": [
        {
            "name": "group by 1",
            "settings": [
                { "num_groups": 144 }
            ]
        },
        {
            "name": "group by 2",
            "settings": [
                { "num_groups": 72 }
            ]
        },
        {
            "name": "group by 4",
            "settings": [
                { "num_groups": 36 }
            ]
        },
        {
            "name": "group by 8",
            "settings": [
                { "num_groups": 18 }            
            ]
        }
        ]
    }
    
    )")
    {
        addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (N);
    }

    void wifiAnimation() override
    {
        // Traits::OutputInterface* o = m_outputs[0];
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