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
		uint16_t m_numLeds;
		uint8_t m_numPxls = 3U;
	
	public:
		template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
		void bindTemplate()
		{
			cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB *)m_ledBuffer, m_numLeds);
			m_usedPins.emplace(TPin);
		};

		HardwareLED(uint16_t num_leds)
			: m_numLeds(num_leds)
			{}

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
