#pragma once
#include <cstdint>
#include "Core/Hardware/WS2815.h"
#include "Traits/Outputs.h"
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Output {

    class TestHardwareLED : public Traits::OutputInterface
    {
    protected:
        uint16_t m_numLeds;
        uint8_t m_numPxls = 3U;

        // CLEDController* m_cled;
        Core::WS2815 ws2815;
    public:

        TestHardwareLED(uint16_t num_leds, int8_t TPin)
            : Traits::OutputInterface(num_leds * m_numPxls), m_numLeds(num_leds)
        {
            if(!Handler::PinHandler::available(TPin))
            {
                Utils::Logger::printf("Error adding fastled: pin %d already used\n", TPin);
                return;
            }

            ws2815.init(num_leds, TPin);
            this->m_dstBuffer = ws2815.getRaw();
            // m_cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) m_dstBuffer, m_numLeds);
            // m_cled->clearLeds();
        }

		void bind() override
		{
		}

        void postupdate() override {
            ws2815.show();
        }

        void setPreset(const JsonDocument& doc) override {

        }

        void applyDimmer(float percentage) override
        {
            for(uint16_t i = 0; i < m_numLeds * m_numPxls; i++)
            {
                m_dstBuffer[i] = static_cast<uint8_t>(255.f * percentage);
            }
        }
    };
}