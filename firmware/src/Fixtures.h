#pragma once
#include "Fixture/Fixture.h"
#include "Output/HardwareLED1D.h"


template<uint8_t Pin = 5>
class Strip1m60 : public Fixture
{
	std::string presets =
		R"(
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
		
		)";
public:
	Strip1m60()
		: Fixture("strip1m", "led_strip_60", presets)
    {
        addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (60);
    }

	Strip1m60(std::string name, std::string type)
		: Fixture(name, type, presets)
    {
        addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (60);
    }

    void wifiAnimation() override
    {
        static uint16_t off = 0;
        getSrcBuffer()[off] = 255;
        getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
        off = (off + 1) % 180;
        vTaskDelay(20);
    }
};

template<uint8_t Pin = 5>
class Strip1m144 : public Fixture
{
	std::string presets = 
		R"({
			  "groups": [
				{
				  "name": "group by 1",
				  "settings": [
					[
					  { "num_groups": 144 }
					]
				  ]
				},
				{
				  "name": "group by 2",
				  "settings": [
					[
					  { "num_groups": 72 }
					]
				  ]
				},
				{
				  "name": "group by 4",
				  "settings": [
					[
					  { "num_groups": 36 }
					]
				  ]
				},
				{
				  "name": "group by 8",
				  "settings": [
					[
					  { "num_groups": 16 }
					]
				  ]
				}
			  ]
			}
			
			)";
public:
	Strip1m144()
		: Fixture("strip1m144", "led_strip_144", presets)
    {
        addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (144);
    }

	Strip1m144(std::string name, std::string type)
		: Fixture(name, type, presets)
    {
        addOutput<Output::HardwareLED1D<WS2815, Pin, RGB>> (144);
    }

    void wifiAnimation() override
    {
        static uint16_t off = 0;
        getSrcBuffer()[off] = 255;
        getSrcBuffer()[off == 0 ? (144*3 - 1) : off - 1] = 0;
        off = (off + 1) % 180;
        vTaskDelay(20);
    }
};


struct Astera60 : public Strip1m60<5>
{
	Astera60() : Strip1m60<5>("neko ime", "astera60"){}
};

struct Astera144 : public Strip1m144<5>
{
	Astera144() : Strip1m144<5>("neko ime", "astera144"){}
};

