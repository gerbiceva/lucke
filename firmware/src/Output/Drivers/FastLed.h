#pragma once
#include <memory>
#include <span>
#include <FastLED.h>
#include "Output/Driver.h"
#include "Hardware/WS2815.h"      // Core::RGB
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Drivers
{
    static_assert(sizeof(Core::RGB) == sizeof(CRGB), "pixel layouts must match");

    class FastLedDevice final : public Output::Driver
    {
    public:
        FastLedDevice(CLEDController& controller, int pin)
            : m_controller(controller), m_pin(pin) {}

        // Flushes only this controller. FastLED.show() would send every
        // FastLED pin, so two strips meant two full sends per frame.
        void show() override { m_controller.showLeds(FastLED.getBrightness()); }

        const char* kind() const override { return "FastLED"; }
        int pin() const override { return m_pin; }

    private:
        CLEDController& m_controller;   // owned by FastLED, lives until reboot
        int m_pin;
    };

    template <template <uint8_t, EOrder> class Chip, EOrder Order = GRB>
    struct FastLed
    {
        static constexpr const char* kind = "FastLED";
        static constexpr Utils::Log s_log{"FASTLED"};

        template <uint8_t Pin>
        static std::unique_ptr<Output::Driver> make(std::span<Core::RGB> pixels)
        {
            if (!Handler::Pin::available(Pin))
            {
                s_log.error("pin {} already used", Pin);
                return nullptr;
            }

            CLEDController& controller = FastLED.addLeds<Chip, Pin, Order>(
                reinterpret_cast<CRGB*>(pixels.data()), static_cast<int>(pixels.size()));
            controller.clearLeds();
            return std::make_unique<FastLedDevice>(controller, Pin);
        }
    };
}