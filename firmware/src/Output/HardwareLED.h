#pragma once

#include "FastLED.h"
#include <string>
#include <set>

namespace Output
{

	class HardwareLED
	{
		static std::set<uint8_t> m_usedPins;
	protected:
		CLEDController *cled;
		uint8_t m_ledBuffer[512];
	
		template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
		void bindTemplate(uint16_t numLeds)
		{
			cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB *)m_ledBuffer, numLeds);
			m_usedPins.emplace(TPin);
		};
	public:

		uint8_t* ledBuffer()
		{
			return m_ledBuffer;
		}
		
		// virtual void bind() = 0;
	
		static void updateFastLED()
		{
			FastLED.show();
		}
		
	};
}
