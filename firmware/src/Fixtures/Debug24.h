#pragma once
#include "Strip2815.h"

namespace Fixtures {

    struct Debug24 : public Strip2815<24>
    {

        Debug24(std::string name = "name")
            : Strip2815(name, "Debug24",  R"(
        {
            "groups": [
            {
                "name": "group by 1",
                "settings": [
                    { "num_groups": 24 }
                ]
            },
            {
                "name": "group by 2",
                "settings": [
                    { "num_groups": 12 }
                ]
            },
            {
                "name": "group by 4",
                "settings": [
                    { "num_groups": 6 }
                ]
            },
            {
                "name": "group by 8",
                "settings": [
                    { "num_groups": 3 }            
                ]
            }
            ]
        }
        
        )")
        {
        }
    };
}
