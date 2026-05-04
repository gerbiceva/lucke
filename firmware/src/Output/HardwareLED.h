#pragma once
#include <cstdint>
#include <FastLED.h>
#include "Traits/Outputs.h"
#include "Core/Handlers/PinHandler.h"
#include "Utils/Logger.h"
#include "Core/Hardware/WS2815.h"

namespace Output {

    // namespace Hardware {
    //     struct Iface
    //     {
    //         virtual void init(uint16_t numLeds, uint8_t pin, uint8_t* dstBuffer) = 0;
    //         virtual void show() = 0;
    //     };
    //
    //     template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    //     struct FastLEDIface : Iface {
    //         CLEDController* m_cled;
    //
    //         // static CLEDController* create() {
    //         void init(uint16_t numLeds, uint8_t pin, uint8_t* dstBuffer) override {
    //             m_cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) dstBuffer, numLeds);
    //         }
    //
    //         void show() override {
    //             FastLED.show();
    //         }
    //     };
    //
    //     struct CustomIface : Iface {
    //         Core::WS2815 ws2815;
    //
    //         void init(uint16_t numLeds, uint8_t pin, uint8_t* dstBuffer) override {
    //             ws2815.init(numLeds, pin, dstBuffer);
    //             // dstBuffer
    //         }
    //
    //         void show() override {
    //             ws2815.show();
    //         }
    //     };
    // }



    // void updateFastLED();



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
            if(!Handler::Pin::available(TPin))
            {
                Utils::Logger::printf("Error adding fastled: pin %d already used\n", TPin);
                return;
            }

            m_cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) m_dstBuffer, m_numLeds);
            m_cled->clearLeds();
            // updateFastLED();
        }

        void bind() override {}
        void setPreset(const JsonDocument &doc) override {}

        void postupdate() override {
            FastLED.show();
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