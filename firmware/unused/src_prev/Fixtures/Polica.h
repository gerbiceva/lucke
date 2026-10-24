#pragma once
#include "Generics/Strip2815.h"

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