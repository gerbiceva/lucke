#pragma once
#include <cstdint>
#include <memory>
#include <FastLED.h>
#include "Base.h"
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Output
{
template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
class FastLEDBase : public Base
{
protected:
    static const uint8_t c_numPxls = 3U;
    uint16_t m_numLeds;

    std::unique_ptr<uint8_t[]> m_dstBuffer;
    CLEDController* m_cled;
public:

    FastLEDBase(uint16_t num_leds)
        : Base(num_leds * c_numPxls), m_numLeds(num_leds)
    {
        if(!Handler::Pin::available(TPin))
        {
            Utils::Logger::printf("Error adding fastled: pin %d already used\n", TPin);
            return;
        }

        m_dstBuffer.reset(new uint8_t[m_numLeds * c_numPxls]());
        this->m_cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) m_dstBuffer.get(), m_numLeds);
        this->m_cled->clearLeds();
    }

    void bind() override {}
    void postupdate() override 
    {
        FastLED.show();
    }

    void appendBasicInfo(JsonDocument& obj) override 
    {
        int nl = this->m_numLeds;
        if (obj["num_leds"].is<int>()) {
            nl += obj["num_leds"].as<int>();
        }
        
        obj["num_leds"] = nl;
    };
};    
} // namespace Output
