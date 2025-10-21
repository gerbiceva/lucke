#pragma once
#include "Fixture/Fixture1D.h"
#include "Fixture/Fixture2D.h"

template<uint8_t TPin>
struct Astera : public Fixture 
{
	Astera()
		: Lamp("Astera", "Astera60", 60)
	{
		bindTemplate<WS2815, TPin, RGB>(m_numLeds);
	}
};

template<uint8_t TPin>
struct Astera144 : public Fixture 
{
	Astera()
		: Lamp("Astera", "Astera144", 144)
	{
		bindTemplate<WS2815, TPin, RGB>(m_numLeds);
	}
};
