//
// Created by Primoz Lavric on 27. 12. 25.
//

#pragma once
#include <cstdint>
#include <FastLED.h>
#include "Traits/Outputs.h"
#include "Core/Handlers/PinHandler.h"
#include "Utils/Logger.h"


namespace OutputV2 {

template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
struct LEDManifest {
  static constexpr uint8_t Pin = TPin;
  static constexpr EOrder Order = TOrder;

  static CLEDController& create(uint8_t *data, int nLedsOrOffset, int nLedsIfOffset) {
    if(!Handler::Pin::available(TPin))
    {
      Utils::Logger::printf("Error adding fastled: pin %d already used\n", TPin);
      assert(false);
    }

    return FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) data, nLedsOrOffset, nLedsIfOffset);
  }
};

void updateFastLED();


struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

class LEDStrip
{
public:
  template <typename LEDManifest>
  LEDStrip(LEDManifest manifest, const uint16_t num_leds)
      : m_pin(LEDManifest::Pin), m_order(LEDManifest::Order), m_numLeds(num_leds)
  {
    m_ledBuffer = new Color[num_leds];

    m_ctrl = &manifest.create(reinterpret_cast<uint8_t*>(m_ledBuffer), m_numLeds, 0);
    m_ctrl->clearLeds();
    updateFastLED();
  }

  ~LEDStrip() {
    delete m_ledBuffer;
  }

  uint16_t count() const {
    return m_numLeds;
  }

  // Override operator[] for non-const access (write)
  Color& operator[](const uint16_t index) {
    return m_ledBuffer[index];
  }

  // Override operator[] for const access (read-only)
  const Color& operator[](size_t index) const {
    return m_ledBuffer[index];
  }

  // void applyDimmer(const float percentage)
  // {
  //   for(uint16_t i = 0; i < m_numLeds * m_perLedSize; i++)
  //   {
  //     m_ledBuffer[i] = static_cast<uint8_t>(255.f * percentage);
  //   }
  // }

private:
  static constexpr uint8_t k_bytesPerLED = 3U;

  uint8_t m_pin;
  EOrder m_order;
  uint16_t m_numLeds;
  Color* m_ledBuffer;

  CLEDController* m_ctrl;
};

}

