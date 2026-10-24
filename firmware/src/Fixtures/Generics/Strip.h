#pragma once
#include <algorithm>
#include <array>
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

    template <FixedString Type, typename TDriver, uint16_t N, uint8_t Pin>
    struct Strip : public Fixture
    {
        static constexpr uint16_t BYTES = N * 3;

        Strip() : Fixture("name", Type.value, Presets::c_groupPresets<N>.data())
        {
            addOutput<Output::Strip1D<TDriver, Pin>>(N);
        }

        // Unchanged from StripWS2815 until the generic Effect replaces it.
        void wifiAnimation() override
        {
            {
                // A preset change from the command task resizes the outputs.
                auto guard = lock();
                m_animBuf[m_animOff] = 255;
                m_animBuf[m_animOff == 0 ? (BYTES - 1) : m_animOff - 1] = 0;
                m_animOff = (m_animOff + 1) % BYTES;

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
        std::array<uint8_t, BYTES> m_animBuf{};
        uint16_t m_animOff = 0;
    };

    // The names main.cpp uses. Type strings match what NVS already stores.
    template <uint16_t N, uint8_t Pin = 5>
    using StripWS2815 = Strip<"StripWS2815", Drivers::Ws2815, N, Pin>;

    template <uint16_t N, uint8_t Pin>
    using StripWS2812 = Strip<"StripWS2812", Drivers::FastLed<WS2812B, GRB>, N, Pin>;

    template <uint16_t N, uint8_t Pin>
    using StripSK6812 = Strip<"StripSK6812", Drivers::FastLed<SK6812, GRB>, N, Pin>;
}