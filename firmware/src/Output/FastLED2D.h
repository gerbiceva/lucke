#pragma once
#include "FastLEDBase.h"
#include "Utils/JsonUtils.h"

namespace Output
{
    template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class FastLED2D : public FastLEDBase<TLedType, TPin, TOrder>
    {
        uint16_t m_width;
        uint16_t m_height;
        uint16_t m_xOffset = 0;
        uint16_t m_yOffset = 0;
        bool m_serpentine = true;

    public:
        FastLED2D(uint16_t num_leds)
            : FastLEDBase<TLedType, TPin, TOrder>(num_leds),
              m_width(num_leds), m_height(1)
        {
        }

        void update(uint8_t* srcbuff) override
        {
            for (uint16_t y = 0; y < m_height; y++) {
                for (uint16_t x = 0; x < m_width; x++) {
                    uint16_t srcIndex = (y * m_width + x) * this->c_numPxls;
                    uint16_t ledIndex;

                    if (m_serpentine && (y & 1)) {
                        ledIndex = (y * m_width + (m_width - 1 - x)) * this->c_numPxls;
                    } else {
                        ledIndex = (y * m_width + x) * this->c_numPxls;
                    }

                    for (uint16_t k = 0; k < this->c_numPxls; k++) {
                        this->m_dstBuffer.get()[ledIndex + k] = srcbuff[srcIndex + k];
                    }
                }
            }
        }

        void setPreset(const JsonDocument& doc) override
        {
            if (!Utils::Json::updateElement<uint16_t>(doc, "width", m_width)) {
                Utils::Logger::println("[HLED_2D] json does not contain 'width'");
            }
            if (!Utils::Json::updateElement<uint16_t>(doc, "height", m_height)) {
                Utils::Logger::println("[HLED_2D] json does not contain 'height'");
            }
            Utils::Json::updateElement<uint16_t>(doc, "x_offset", m_xOffset);
            Utils::Json::updateElement<uint16_t>(doc, "y_offset", m_yOffset);
            Utils::Json::updateElement<bool>(doc, "serpentine", m_serpentine);
        }

        uint16_t footprint() const override
        {
            return m_width * m_height * this->c_numPxls;
        }

        void toJson(JsonObject& doc) override
        {
            doc["uuid"] = this->m_uuid;
            doc["type"] = "FastLED-2D";
            doc["hardware_pin"] = TPin;
            doc["num_leds"] = this->m_numLeds;
            doc["width"] = m_width;
            doc["height"] = m_height;
            doc["x_offset"] = m_xOffset;
            doc["y_offset"] = m_yOffset;
            doc["serpentine"] = m_serpentine;
        }
    };
}
