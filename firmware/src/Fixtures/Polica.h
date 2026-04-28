#pragma once
#include "Strip2815.h"

// struct Polica : public Fixture
// {
//     static constexpr uint16_t N = 70U;
//     static constexpr uint16_t BYTES = N * 3;

//     Polica()
// 		: Fixture("name", "kasper-polica",  R"(
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
//         addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (N);
//     }

//      void wifiAnimation() override
//     {
//         static uint16_t off = 0;
//         getSrcBuffer()[off] = 255;
//         getSrcBuffer()[off == 0 ? (BYTES - 1) : off - 1] = 0;
//         off = (off + 1) % BYTES;
//         vTaskDelay(20);
//     }

//     void highlight() override
//     {
//         memset(getOffsetSrcBuffer(), 255, BYTES);
//     }
// };

namespace Fixtures
{

struct Polica : public Strip2815<70>
{
    Polica(std::string name = "name")
		: Strip2815("name", "kasper-polica",  R"(
    {
        "groups": [
        {
            "name": "group by 1",
            "settings": [
                { "num_groups": 70 }
            ]
        },
        {
            "name": "group by 2",
            "settings": [
                { "num_groups": 35 }
            ]
        },
        {
            "name": "group by 5",
            "settings": [
                { "num_groups": 14 }
            ]
        },
        {
            "name": "group by 10",
            "settings": [
                { "num_groups": 7 }            
            ]
        }
        ]
    }
    
    )")
    {
    }
};
}