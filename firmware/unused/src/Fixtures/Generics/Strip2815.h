#pragma once
#include <Arduino.h>
#include "Core/Fixture.h"
#include "Utils/PresetMaker.h"
#include "Output/FastLED1D.h"

namespace Fixtures
{

    template <uint16_t N, uint8_t TPin = 5>
    struct Strip2815 : public Fixture
    {
        static constexpr uint16_t BYTES = N * 3;

        Strip2815()
            : Fixture("name", "Strip2815", MakeJSON<N>::make().data())
        {
            addOutput<Output::FastLED1D<WS2815, TPin, RGB>>(N);
        }

        Strip2815(std::string name, std::string type, std::string presetJson)
            : Fixture(name, type, presetJson)
        {
            addOutput<Output::FastLED1D<WS2815, TPin, RGB>>(N);
        }

        void wifiAnimation() override
        {
            m_animBuf[m_animOff] = 255;
            m_animBuf[m_animOff == 0 ? (BYTES - 1) : m_animOff - 1] = 0;
            m_animOff = (m_animOff + 1) % BYTES;

            uint8_t *ptr = m_animBuf;
            for (auto o : getOutputs())
            {
                o->update(ptr);
                ptr += o->footprint();
            }

            vTaskDelay(20);
        }

    private:
        uint8_t m_animBuf[BYTES] = {};
        uint16_t m_animOff = 0;
    };

}
