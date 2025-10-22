#pragma once
#include "Fixture/Fixture1D.h"
#include "Fixture/Fixture2D.h"

template<uint8_t TPin>
struct Astera : public Fixture1D
{
	Astera()
		: Fixture1D("Astera", "Astera60", 60,
		std::vector<Preset1D>{
			{"", 1},
			{"", 1},
			{"", 1},
			{"", 1}
		})
	{
		bindTemplate<WS2815, TPin, RGB>(m_numLeds);
	}
};

template<uint8_t TPin>
struct Astera144 : public Fixture1D
{
	Astera144()
		: Fixture1D("Astera", "Astera144", 144, 
		std::vector<Preset1D>{
			{"", 1},
			{"", 1},
			{"", 1},
			{"", 1}		
		})
	{
		bindTemplate<WS2815, TPin, RGB>(m_numLeds);
	}
};
