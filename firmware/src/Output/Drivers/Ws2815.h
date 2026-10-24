#pragma once
#include <memory>
#include <span>
#include "Output/Driver.h"
#include "Hardware/WS2815.h"
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Drivers
{
    class Ws2815Device final : public Output::Driver
    {
    public:
        explicit Ws2815Device(int pin) : m_pin(pin) {}

        bool init(std::span<Core::RGB> pixels)
        {
            std::span<uint8_t> bytes{reinterpret_cast<uint8_t*>(pixels.data()), pixels.size_bytes()};
            return m_dev.init(m_pin, bytes) == ESP_OK;
        }

        void show() override { m_dev.show(); }
        const char* kind() const override { return "WS2815"; }
        int pin() const override { return m_pin; }

    private:
        Core::WS2815 m_dev;
        int m_pin;
    };

    // Policy used as Strip1D's TDriver. Your library takes the pin at
    // runtime; it's a template parameter here only so both drivers share
    // one signature.
    struct Ws2815
    {
        static constexpr const char* kind = "WS2815";
        static constexpr Utils::Log s_log{"WS2815"};

        template <uint8_t Pin>
        static std::unique_ptr<Output::Driver> make(std::span<Core::RGB> pixels)
        {
            if (!Handler::Pin::claim(Pin))
            {
                s_log.error("pin %u already used", Pin);
                return nullptr;
            }

            auto dev = std::make_unique<Ws2815Device>(Pin);
            if (!dev->init(pixels))
            {
                // The lib owns SPI2_HOST, so a second strip lands here.
                s_log.error("init on pin %u failed (only one WS2815 strip is supported)", Pin);
                return nullptr;
            }
            return dev;
        }
    };
}