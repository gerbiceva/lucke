#pragma once
#include <cstdint>
#include <FastLED.h>
#include "Traits/Outputs.h"
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Output {

    void updateFastLED();

    template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class HardwareLED : public Traits::OutputInterface
    {
    protected:
        uint16_t m_numLeds;
        uint8_t m_numPxls = 3U;

        CLEDController* m_cled;
    public:

        HardwareLED(uint16_t num_leds)
            : Traits::OutputInterface(num_leds * m_numPxls), m_numLeds(num_leds)
        {
        }

		void bind() override
		{
            if(!Handler::PinHandler::available(TPin))
            {
                Utils::Logger::printf("Error adding fastled: pin %d already used\n", TPin);
                return;
            }

            m_cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) m_dstBuffer, m_numLeds);
            m_cled->clearLeds();
            updateFastLED();
		};

        void applyDimmer(float percentage) override
        {
            for(uint16_t i = 0; i < m_numLeds * m_numPxls; i++)
            {
                m_dstBuffer[i] = static_cast<uint8_t>(255.f * percentage);
            }
        }
    };
}