#pragma once
#include "Strip2815.h"

namespace Fixtures {

struct KasperDiffused : public Strip2815<288>
{

    KasperDiffused(std::string name = "name")
		: Strip2815(name, "KasperDiffused",  R"(
    {
        "groups": [
        {
            "name": "group by 2",
            "settings": [
                { "num_groups": 144 }
            ]
        },
        {
            "name": "group by 4",
            "settings": [
                { "num_groups": 72 }
            ]
        },
        {
            "name": "group by 8",
            "settings": [
                { "num_groups": 36 }
            ]
        },
        {
            "name": "group by 16",
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
