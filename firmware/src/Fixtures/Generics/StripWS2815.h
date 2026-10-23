#pragma once
#include <Arduino.h>
#include <array>
#include <span>
#include "Core/Fixture.h"
#include "Utils/PresetMaker.h"
#include "Output/LED1D.h"

namespace Fixtures
{

    // Same as Strip2815 but driven by the native WS2815 SPI output
    // (Output::WS2815_1D) instead of FastLED.
    template <uint16_t N, uint8_t TPin = 5>
    struct StripWS2815 : public Fixture
    {
        static constexpr uint16_t BYTES = N * 3;

        StripWS2815()
            : Fixture("name", "StripWS2815", MakeJSON<N>::make().data())
        {
            addOutput<Output::WS2815_1D>(N, TPin);
        }

        StripWS2815(std::string name, std::string type, std::string presetJson)
            : Fixture(std::move(name), std::move(type), std::move(presetJson))
        {
            addOutput<Output::WS2815_1D>(N, TPin);
        }

        void wifiAnimation() override
        {
            m_animBuf[m_animOff] = 255;
            m_animBuf[m_animOff == 0 ? (BYTES - 1) : m_animOff - 1] = 0;
            m_animOff = (m_animOff + 1) % BYTES;

            std::span<const uint8_t> src{m_animBuf};
            for (auto& o : getOutputs())
            {
                o->update(src.first(o->footprint()));
                src = src.subspan(o->footprint());
            }

            vTaskDelay(20);
        }

    private:
        std::array<uint8_t, BYTES> m_animBuf{};
        uint16_t m_animOff = 0;
    };

}
