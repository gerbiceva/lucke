#pragma once
#include "Core/Fixture.h"
#include "Output/HardwareLED1D.h"
#include "Output/HardwareLED2D.h"
#include "Utils/PresetMaker.h"

template<uint16_t N>
struct Strip : public Fixture
{
    static constexpr uint16_t BYTES = N * 3;

    Strip()
		: Fixture("neki", "Strip",  MakeJSON<N>::make().data())
    {
        addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (N);

    }

    void wifiAnimation() override
    {
        static uint16_t off = 0;
        getOffsetSrcBuffer()[off] = 255;
        getOffsetSrcBuffer()[off == 0 ? (BYTES - 1) : off - 1] = 0;
        off = (off + 1) % BYTES;
        vTaskDelay(20);
    }

    void highlight() override
    {
        memset(getOffsetSrcBuffer(), 255, BYTES);
    }
};

template<uint8_t TPin>
struct Christmas : public Fixture
{
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
        addOutput<Output::HardwareLED1D<WS2812B, TPin, GRB>> (80);
    }

    void wifiAnimation() override
    {
        // Traits::OutputInterface* o = m_outputs[0];
        static uint16_t off = 0;
        constexpr uint16_t N = (80U * 3U);
        getSrcBuffer()[off] = 255;
        getSrcBuffer()[off == 0 ? (N - 1) : off - 1] = 0;
        off = (off + 1) % N;
        vTaskDelay(20);
    }

    void highlight() override
    {
        memset(getOffsetSrcBuffer(), 255, 80 * 3);
    }
};




// struct Astera145 : public Fixture
// {
//     Astera145()
// 		: Fixture("neki", "Christmas",  R"(
//     {
//         "groups": [
//         {
//             "name": "group by 1",
//             "settings": [
//                 { "num_groups": 80 }
//             ]
//         },
//         {
//             "name": "group by 2",
//             "settings": [
//                 { "num_groups": 40 }
//             ]
//         },
//         {
//             "name": "group by 4",
//             "settings": [
//                 { "num_groups": 20 }
//             ]
//         },
//         {
//             "name": "group by 8",
//             "settings": [
//                 { "num_groups": 10 }            
//             ]
//         }
//         ]
//     }
    
//     )")
//     {
//         addOutput<Output::HardwareLED1D<WS2812B, 6, GRB>> (80);
//     }

//     void wifiAnimation() override
//     {
//         // Traits::OutputInterface* o = m_outputs[0];
//         static uint16_t off = 0;
//         constexpr uint16_t N = (80U * 3U);
//         getSrcBuffer()[off] = 255;
//         getSrcBuffer()[off == 0 ? (N - 1) : off - 1] = 0;
//         off = (off + 1) % N;
//         vTaskDelay(20);
//     }

//     void highlight() override
//     {
//         memset(getOffsetSrcBuffer(), 255, 80 * 3);
//     }
// };
// struct Astera60 : public Fixture
// {
//     Astera60()
// 		: Fixture("neki", "Strip",  R"(
//     {
    //     "groups": [
    //     {
    //         "name": "group by 1",
    //         "settings": [
            
    //             { "num_groups": 60 },            
    //         ]
    //     },
    //     {
    //         "name": "group by 2",
    //         "settings": [
            
    //             { "num_groups": 30 }
            
    //         ]
    //     },
    //     {
    //         "name": "group by 4",
    //         "settings": [
            
    //             { "num_groups": 15 }
            
    //         ]
    //     },
    //     {
    //         "name": "group by 10",
    //         "settings": [
            
    //             { "num_groups": 6 }
            
    //         ]
    //     }
    //     ]
    // }
    
//     )")
//     {
// 	    // OutputV2::HardwareStrip(OutputV2::LEDManifest<WS2815, 5, RGB>{}, 60);
//         addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (60);
//     }

//     void wifiAnimation() override
//     {
//         static uint16_t off = 0;
//         getOffsetSrcBuffer()[off] = 255;
//         getOffsetSrcBuffer()[off == 0 ? (60*3 - 1) : off - 1] = 0;
//         off = (off + 1) % 180;
//         vTaskDelay(20);
//     }

//     void highlight() override
//     {
//         // uint8_t* buff = getOffsetSrcBuffer();
//         memset(getOffsetSrcBuffer(), 255, 60*3);
        
//         // constexpr uint8_t ledCount = 60;

//         // for(uint8_t i = 0; i < ledCount; i++)
//         // {
//         //     float t = (float)i / (ledCount - 1);
//         //     uint8_t value = (uint8_t)(255.0f * (1.0f - t));
//         //     uint16_t index = i * 3;
//         //     buff[index + 0] = value;
//         //     buff[index + 1] = value;
//         //     buff[index + 2] = value;
//         // }
//     }
// };


// struct Astera144 : public Fixture
// {
//     Astera144()
// 		: Fixture("neki", "Astera144",  R"(
//     {
//         "groups": [
//         {
//             "name": "group by 1",
//             "settings": [
//                 { "num_groups": 144 }
//             ]
//         },
//         {
//             "name": "group by 2",
//             "settings": [
//                 { "num_groups": 72 }
//             ]
//         },
//         {
//             "name": "group by 4",
//             "settings": [
//                 { "num_groups": 36 }
//             ]
//         },
//         {
//             "name": "group by 8",
//             "settings": [
//                 { "num_groups": 18 }            
//             ]
//         }
//         ]
//     }
    
//     )")
//     {
//         addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (144);
//     }

//     void wifiAnimation() override
//     {
//         // Traits::OutputInterface* o = m_outputs[0];
//         static uint16_t off = 0;
//         constexpr uint16_t N = (144U * 3U);
//         getSrcBuffer()[off] = 255;
//         getSrcBuffer()[off == 0 ? (N - 1) : off - 1] = 0;
//         off = (off + 1) % N;
//         vTaskDelay(20);
//     }
// };



// struct Polica : public Fixture
// {
//     Polica()
// 		: Fixture("kasper-polica2", "Strip",  R"(
//     {
//         "groups": [
//         {
//             "name": "group by 1",
//             "settings": [
//                 { "num_groups": 70 }
//             ]
//         },
//         {
//             "name": "group by 2",
//             "settings": [
//                 { "num_groups": 35 }
//             ]
//         },
//         {
//             "name": "group by 5",
//             "settings": [
//                 { "num_groups": 14 }
//             ]
//         },
//         {
//             "name": "group by 10",
//             "settings": [
//                 { "num_groups": 7 }            
//             ]
//         }
//         ]
//     }
    
//     )")
//     {
//         addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (70);
//     }

//     void wifiAnimation() override
//     {
//         // Traits::OutputInterface* o = m_outputs[0];
//         static uint16_t off = 0;
//         constexpr uint16_t N = (70U * 3U);
//         getSrcBuffer()[off] = 255;
//         getSrcBuffer()[off == 0 ? (N - 1) : off - 1] = 0;
//         off = (off + 1) % N;
//         vTaskDelay(20);
//     }
// };

// struct SGMQ1 : public Fixture
// {
//     SGMQ1()
// 		: Fixture("neki", "SGMQ1",  R"(
    // {
    //     "groups": [
    //     {
    //         "name": "12x8",
    //         "settings": [
    //             { "column_width": 2, "row_height": 2 }
    //         ]
    //         },
    //     {
    //         "name": "6x4",
    //         "settings": [
    //             { "column_width": 4, "row_height": 4 }            
    //         ]
    //     },
    //     {
    //         "name": "6x2",
    //         "settings": [
    //             { "column_width": 4, "row_height": 8 }            
    //         ]
    //     },
    //     {
    //         "name": "6x1",
    //         "settings": [
    //             { "column_width": 4, "row_height": 16 }            
    //         ]
    //     }
    //     ]
    // }
//     )")
//     {
//         addOutput<Output::HardwareLED2D<WS2812, 5, GRB>> (8U, 24U);
//         // addOutput<Output::HardwareLED2D<WS2812, 6, GRB>> (8U, 24U);
//         // addOutput<Output::HardwareLED1D<WS2812, 6, GRB>> (8U, 24U);
//         // addOutput<Output::VirtualGrid<WS2812, 5, GRB>> (8U, 24U);
//     }

//     ~SGMQ1() override = default;

//     void wifiAnimation() override
//     {
//         // static uint16_t off = 0;
//         // constexpr uint16_t N = (144U * 3U);
//         // getSrcBuffer()[off] = 255;
//         // getSrcBuffer()[off == 0 ? (N - 1) : off - 1] = 0;
//         // off = (off + 1) % N;
//         // vTaskDelay(20);
//     }
// };

