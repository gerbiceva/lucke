#pragma once
#include <algorithm>
#include <vector>
#include <span>
#include "Core/Fixture.h"
#include "Output/Strip1D.h"
#include "Output/Drivers/Ws2815.h"
#include "Output/Drivers/FastLed.h"
#include "Utils/PresetMaker.h"

namespace Fixtures
{
    // A string literal usable as a template argument (C++20).
    template <size_t N>
    struct FixedString
    {
        char value[N];
        constexpr FixedString(const char (&s)[N]) { std::copy_n(s, N, value); }
    };

    // The LED count comes from the stored record, or the default passed to
    // addFixture (see FixtureHandler::addFixture).
    template <FixedString Type, typename TDriver, uint8_t Pin>
    struct Strip : public Fixture
    {
        explicit Strip(uint16_t numLeds)
            : Fixture("name", Type.value, Presets::groupPresets(numLeds)),
              m_animBuf(numLeds * 3, 0)
        {
            setLedCount(numLeds);
            addOutput<Output::Strip1D<TDriver, Pin>>(numLeds);
        }

        // Unchanged from StripWS2815 until the generic Effect replaces it.
        void wifiAnimation() override
        {
            {
                // A preset change from the command task resizes the outputs.
                auto guard = lock();
                const uint16_t bytes = m_animBuf.size();
                m_animBuf[m_animOff] = 255;
                m_animBuf[m_animOff == 0 ? (bytes - 1) : m_animOff - 1] = 0;
                m_animOff = (m_animOff + 1) % bytes;

                std::span<const uint8_t> src{m_animBuf};
                for (auto& o : getOutputs())
                {
                    o->update(src.first(o->footprint()));
                    src = src.subspan(o->footprint());
                }
            }
            vTaskDelay(20);
        }

    private:
        std::vector<uint8_t> m_animBuf;
        uint16_t m_animOff = 0;
    };

    // The names main.cpp uses. Type strings match what NVS already stores.
    template <uint8_t Pin = 5>
    using StripWS2815 = Strip<"StripWS2815", Drivers::Ws2815, Pin>;

    template <uint8_t Pin>
    using StripWS2812 = Strip<"StripWS2812", Drivers::FastLed<WS2812B, GRB>, Pin>;

    template <uint8_t Pin>
    using StripSK6812 = Strip<"StripSK6812", Drivers::FastLed<SK6812, GRB>, Pin>;
}