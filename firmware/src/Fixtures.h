#pragma once
#include "Core/Fixture.h"
#include "Output/HardwareLED1D.h"
#include "Output/HardwareLED2D.h"

struct Astera60 : public Fixture
{
    // std::string presets =
//    ;
public:
    Astera60()
		: Fixture("neki", "Astera60",  R"(
    {
        "groups": [
        {
            "name": "group by 1",
            "settings": [
            [
                { "num_groups": 60 }
            ]
            ]
        },
        {
            "name": "group by 2",
            "settings": [
            [
                { "num_groups": 30 }
            ]
            ]
        },
        {
            "name": "group by 4",
            "settings": [
            [
                { "num_groups": 15 }
            ]
            ]
        },
        {
            "name": "group by 10",
            "settings": [
            [
                { "num_groups": 6 }
            ]
            ]
        }
        ]
    }
    
    )")
    {
        addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (60);
    }

    void wifiAnimation() override
    {
        static uint16_t off = 0;
        getSrcBuffer()[off] = 255;
        getSrcBuffer()[off == 0 ? (60*3 - 1) : off - 1] = 0;
        off = (off + 1) % 180;
        vTaskDelay(20);
    }
    // using Astera60 = Fixture;
};


struct Kitajci200 : public Fixture
{
    // std::string presets =
//    ;
public:
    Kitajci200()
		: Fixture("neki", "Kitajci200",  R"(
    {
        "groups": [
        {
            "name": "group by 1",
            "settings": [
            [
                { "num_groups": 60 }
            ]
            ]
        },
        {
            "name": "group by 2",
            "settings": [
            [
                { "num_groups": 30 }
            ]
            ]
        },
        {
            "name": "group by 4",
            "settings": [
            [
                { "num_groups": 15 }
            ]
            ]
        },
        {
            "name": "group by 10",
            "settings": [
            [
                { "num_groups": 6 }
            ]
            ]
        }
        ]
    }
    
    )")
    {
        addOutput<Output::HardwareLED1D<WS2812, 5, RGB>> (170);
    }

    void wifiAnimation() override
    {
        static uint16_t off = 0;
        getSrcBuffer()[off] = 255;
        getSrcBuffer()[off == 0 ? (170*3 - 1) : off - 1] = 0;
        off = (off + 1) % (170*3);
        vTaskDelay(20);
    }
};

// template<uint8_t Pin = 5>
// class Strip1m60 : public Fixture
// {
// 	std::string presets =
// 		R"(
// 		{
// 		  "groups": [
// 		    {
// 		      "name": "group by 1",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 60 }
// 		        ]
// 		      ]
// 		    },
// 		    {
// 		      "name": "group by 2",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 30 }
// 		        ]
// 		      ]
// 		    },
// 		    {
// 		      "name": "group by 4",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 15 }
// 		        ]
// 		      ]
// 		    },
// 		    {
// 		      "name": "group by 10",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 6 }
// 		        ]
// 		      ]
// 		    }
// 		  ]
// 		}
		
// 		)";
// public:
// 	Strip1m60()
// 		: Fixture("strip1m", "led_strip_60", presets)
//     {
//         addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (60);
//     }

// 	Strip1m60(std::string name, std::string type)
// 		: Fixture(name, type, presets)
//     {
//         addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (60);
//     }

//     void wifiAnimation() override
//     {
//         static uint16_t off = 0;
//         getSrcBuffer()[off] = 255;
//         getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
//         off = (off + 1) % 180;
//         vTaskDelay(20);
//     }
// };

// template<uint8_t Pin = 5>
// class Strip1m144 : public Fixture
// {
// 	std::string presets = 
// 		R"({
// 			  "groups": [
// 				{
// 				  "name": "group by 1",
// 				  "settings": [
// 					[
// 					  { "num_groups": 144 }
// 					]
// 				  ]
// 				},
// 				{
// 				  "name": "group by 2",
// 				  "settings": [
// 					[
// 					  { "num_groups": 72 }
// 					]
// 				  ]
// 				},
// 				{
// 				  "name": "group by 4",
// 				  "settings": [
// 					[
// 					  { "num_groups": 36 }
// 					]
// 				  ]
// 				},
// 				{
// 				  "name": "group by 8",
// 				  "settings": [
// 					[
// 					  { "num_groups": 16 }
// 					]
// 				  ]
// 				}
// 			  ]
// 			}
			
// 			)";
// public:
// 	Strip1m144()
// 		: Fixture("strip1m144", "led_strip_144", presets)
//     {
//         addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (144);
//     }

// 	Strip1m144(std::string name, std::string type)
// 		: Fixture(name, type, presets)
//     {
//         addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (144);
//     }

//     void wifiAnimation() override
//     {
//         static uint16_t off = 0;
//         getSrcBuffer()[off] = 255;
//         getSrcBuffer()[off == 0 ? (144*3 - 1) : off - 1] = 0;
//         off = (off + 1) % 180;
//         vTaskDelay(20);
//     }
// };

// // template<uint8_t Pin = 5>
// class Astera60 : public Fixture
// {
// 	std::string presets =
// 		R"(
// 		{
// 		  "groups": [
// 		    {
// 		      "name": "group by 1",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 60 }
// 		        ]
// 		      ]
// 		    },
// 		    {
// 		      "name": "group by 2",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 30 }
// 		        ]
// 		      ]
// 		    },
// 		    {
// 		      "name": "group by 4",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 15 }
// 		        ]
// 		      ]
// 		    },
// 		    {
// 		      "name": "group by 10",
// 		      "settings": [
// 		        [
// 		          { "num_groups": 6 }
// 		        ]
// 		      ]
// 		    }
// 		  ]
// 		}
		
// 		)";
// public:
// 	Astera60()
// 		: Fixture("strip1m", "led_strip_60", presets)
//     {
//         Utils::Logger::println("Adding astera60");
//         addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (60);
//     }

// 	Astera60(std::string name, std::string type)
// 		: Fixture(name, type, presets)
//     {
//         Utils::Logger::println("Adding astera60 params");
//         addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (60);
//     }

//     void wifiAnimation() override
//     {
//         static uint16_t off = 0;
//         getSrcBuffer()[off] = 255;
//         getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
//         off = (off + 1) % 180;
//         vTaskDelay(20);
//     }
// };


// // struct Astera60 : public Strip1m60<5>
// // {
// // 	Astera60() : Strip1m60<5>("neko ime", "astera60"){}
// // };

// struct Astera144 : public Strip1m144<5>
// {
// 	Astera144() : Strip1m144<5>("neko ime", "astera144"){}
// };

// struct Blinder24x8 : public Fixture
// {
// 	Blinder24x8() : Fixture("SgmQ1", "Blinder24x8", 
// 		R"({
// 			"groups": [
// 				{
// 					"name": "12x4 grid",
// 					"settings": 
// 					[
// 						[
// 							{ "column_width": 2, "row_height" : 2 }
// 						]
// 					]
// 				},
// 				{
// 					"name": "4x2 grid",
// 					"settings": 
// 					[
// 						[
// 							{ "column_width": 6, "row_height" : 4 }				  
// 						]
// 					]
// 				},
// 				{
// 					"name": "4x1 grid",
// 					"settings": 
// 					[
// 						[
// 							{ "column_width": 6, "row_height" : 8 }
// 						]
// 					]
// 				},
// 			]
// 		}
// 		)")
// 	{
// 		addOutput<Output::HardwareLED2D<WS2815, 5, RGB>>(24, 8);
// 	}
// };
