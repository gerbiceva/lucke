#include "HardwareLED.h"

namespace OutputV2
{
    void updateFastLED()
    {
        FastLED.show();
    }

    // template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    // void HardwareLED<TLedType, TPin, TOrder>::bind()
    // {
    //     if(!PinHandler::available(TPin))
    //     {
    //         Utils::Logger::printf("Error adding fastled: pin %d already used\n", TPin);
    //         return;
    //     }

    //     // memset(m_dstBuffer, 0, m_size);

    //     m_cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) m_dstBuffer, m_numLeds);
    //     m_cled->clearLeds();
    //     updateFastLED();
    // };
}