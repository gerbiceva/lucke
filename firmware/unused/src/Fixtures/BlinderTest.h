#pragma once
#include "Core/Fixture.h"
#include "Output/FastLEDGrid.h"

struct My8x24Grid : public Output::FastLEDGrid<WS2815, RGB>
{
    My8x24Grid() : FastLEDGrid(8, 3)
    {
        // one strip covering the whole 8x24 grid (8*24 = 192 LEDs).
        // for three physical 8x8 panels instead: addStrip<5>(64,{0,8},{0,8}),
        // addStrip<6>(64,{0,8},{8,16}), addStrip<7>(64,{0,8},{16,24}).
        addStrip<5>(24, {0, 8}, {0, 3});
    }
};

struct GridDemo : public Fixture
{
    // Each preset's width/height is the subgrid resolution (block count) of the grid:
    //   1x3  -> 8x8 cells   (three stacked blinder zones)
    //   2x6  -> 4x4 cells
    //   8x24 -> 1x1 cells   (full resolution)
    GridDemo() : Fixture("name", "BlinderTest", R"({
        "groups": [
            { "name": "8x24", "settings": [ { "width": 8, "height": 3 } ] }
        ]
    })")
    {
        addOutput<My8x24Grid>();
    }
};