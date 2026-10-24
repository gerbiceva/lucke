#pragma once
#include "Generics/Strip2815.h"

namespace Fixtures
{

struct Astera144 : public Strip2815<144>
{
    Astera144()
		: Strip2815("name", "Astera144",  R"(
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
    }
};

}