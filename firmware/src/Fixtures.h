#pragma once
// #include "Fixture/Fixture1D.h"
// #include "Fixture/Fixture2D.h"

// template<uint8_t TPin>
// struct Astera : public Fixture1D
// {
// 	Astera()
// 		: Fixture1D("Astera", "Astera60", 60,
// 		std::vector<Preset1D>{
// 			{"", 1},
// 			{"", 1},
// 			{"", 1},
// 			{"", 1}
// 		})
// 	{
// 		bindTemplate<WS2815, TPin, RGB>(m_numLeds);
// 	}
// };

// template<uint8_t TPin>
// struct Astera144 : public Fixture1D
// {
// 	Astera144()
// 		: Fixture1D("Astera", "Astera144", 144, 
// 		std::vector<Preset1D>{
// 			{"", 1},
// 			{"", 1},
// 			{"", 1},
// 			{"", 1}		
// 		})
// 	{
// 		bindTemplate<WS2815, TPin, RGB>(m_numLeds);
// 	}
// };


#include "Fixture/Fixture.h"
#include "Output/HardwareLED.h"

struct Astera60 : public Fixture
{
	Astera60()
		: Fixture("astera60", "astera60",
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
		
		)")
		{
			addOutput<Output::HardwareLED1D<WS2815, 5, RGB>> (60);
		}
};