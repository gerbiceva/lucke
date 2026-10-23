#pragma once
#include <cstdint>
#include <memory>
#include "Base.h"
#include "Drivers.h"
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Output
{
    // Generic LED output base: owns the destination RGB buffer and a backend
    // driver (FastLED or WS2815). The dimension-specific mapping (1D groups,
    // 2D serpentine, ...) lives in the derived classes; everything backend
    // related is delegated to TDriver.
    template <typename TDriver>
    class LEDBase : public Base
    {
    protected:
        static const uint8_t c_numPxls = 3U;
        uint16_t m_numLeds;
        int m_pin;
        TDriver m_driver;

        std::unique_ptr<uint8_t[]> m_dstBuffer;

    public:
        LEDBase(uint16_t num_leds, int pin)
            : Base(num_leds * c_numPxls), m_numLeds(num_leds), m_pin(pin),
              // Allocated before the pin check so update() always has a
              // valid buffer, even when the driver is never initialized.
              m_dstBuffer(std::make_unique<uint8_t[]>(num_leds * c_numPxls))
        {
            if (pin >= 0 && !Handler::Pin::available(pin))
            {
                Utils::Logger::printf("[LED] pin %d already used\n", pin);
                return;
            }

            m_driver.init(m_numLeds, m_dstBuffer.get(), pin);
        }

        void bind() override {}
        void postupdate() override { m_driver.show(); }

        int hardwarePin() const override { return m_pin; }

        void appendBasicInfo(JsonDocument &obj) override
        {
            int nl = m_numLeds;
            if (obj["num_leds"].is<int>())
            {
                nl += obj["num_leds"].as<int>();
            }
            obj["num_leds"] = nl;
        }

        // num_leds here is the source-pixel count (footprint / 3), derived
        // generically so it stays correct regardless of grouping/mapping.
        void describe(JsonObject &desc) override
        {
            Base::describe(desc);
            desc["num_leds"] = this->footprint() / c_numPxls;
        }
    };
}
