#pragma once
#include "Astera60.h"
#include "Debug24.h"
#include "Polica.h"
#include "KasperDiffused.h"


// // #include "Output/HardwareLED2D.h"
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