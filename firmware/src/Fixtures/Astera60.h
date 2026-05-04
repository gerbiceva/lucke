#pragma once
#include "Generics/Strip2815.h"

namespace Fixtures {

    struct Astera60 : public Strip2815<60>
    {

        Astera60(std::string name = "name")
            : Strip2815(name, "Astera60",  R"(
        {
            "groups": [
            {
                "name": "group by 1",
                "settings": [
                    { "num_groups": 60 }
                ]
            },
            {
                "name": "group by 2",
                "settings": [
                    { "num_groups": 30 }
                ]
            },
            {
                "name": "group by 4",
                "settings": [
                    { "num_groups": 15 }
                ]
            },
            {
                "name": "group by 10",
                "settings": [
                    { "num_groups": 6 }            
                ]
            }
            ]
        }
        
        )")
        {
        }
    };
}
