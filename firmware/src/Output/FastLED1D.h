#pragma once
#include "FastLEDBase.h"
#include "Utils/JsonUtils.h"

namespace Output 
{
    template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class FastLED1D : public FastLEDBase<TLedType, TPin, TOrder>
    {
        uint16_t m_numGroups = 1;
    public:
        FastLED1D(uint16_t num_leds)
            : FastLEDBase<TLedType, TPin, TOrder>(num_leds),
              m_numGroups(num_leds)
        {
        }

        void update(uint8_t* srcbuff) override
        {
            uint16_t groupSize = this->m_numLeds / m_numGroups;
            uint16_t ledIndex = 0;

            for (uint16_t i = 0; i < m_numGroups; i++) {
                for (uint16_t j = 0; j < groupSize; j++) {
                    for (uint16_t k = 0; k < this->c_numPxls; k++) {
                        uint16_t dmxBufferIndex = i * this->c_numPxls + k;
                        this->m_dstBuffer.get()[ledIndex] = srcbuff[dmxBufferIndex];
                        ledIndex++;
                    }
                }
            }
        }

        void setPreset(const JsonDocument& doc) override
        {
            if(!Utils::Json::updateElement<uint16_t>(doc, "num_groups", m_numGroups))
            {
                Utils::Logger::println("[HLED_1D] json does not contain 'num_groups'");
            }
        }

        uint16_t footprint() const override
        {
            return m_numGroups * this->c_numPxls;
        }

        void toJson(JsonObject& doc) override
        {
            doc["uuid"] = this->m_uuid;
            doc["type"] = "FastLED-1D";
            doc["hardware_pin"] = TPin;
            doc["num_leds"] = this->m_numLeds;
            doc["num_groups"] = m_numGroups;
        }
    };
}