#pragma once
#include <cstdint>
#include <FastLED.h>
#include "Traits/Outputs.h"
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"
#include "OutputV2/LEDStrip.h"

namespace OutputV2 {




    void updateFastLED();

    class HardwareLED : public Traits::OutputInterface
    {
    protected:
        uint8_t m_pin;
        EOrder m_order;
        uint16_t m_numLeds;
        uint8_t m_numPxls = 3U;

        CLEDController* m_cled;
    public:
        template <typename LEDManifest>
        HardwareLED(LEDManifest manifest, uint16_t num_leds)
            : Traits::OutputInterface(num_leds * m_numPxls), m_pin(LEDManifest::Pin), m_order(LEDManifest::Order), m_numLeds(num_leds)
        {
            m_cled = &manifest.create(m_dstBuffer, m_numLeds, 0);
            m_cled->clearLeds();
            updateFastLED();
        }

		void bind() override
		{

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