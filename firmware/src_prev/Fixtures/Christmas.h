#pragma once
#include "Generics/Strip2812.h"

template<uint8_t TPin = 5U>
struct Christmas : public Fixtures::Strip2812<80, TPin>
{
    Christmas()
		: Strip2812("neki", "Christmas",  R"(
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
    }
};