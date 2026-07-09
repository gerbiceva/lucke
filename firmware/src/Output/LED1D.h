#pragma once
#include <cstdio>
#include "Base/LEDBase.h"
#include "Utils/JsonUtils.h"

namespace Output
{
    // 1D strip output: maps a source buffer of m_numGroups colors onto
    // m_numLeds physical LEDs (each group repeated across its LEDs).
    // Backend-agnostic via TDriver.
    template <typename TDriver = WS2815Driver>
    class LED1D : public LEDBase<TDriver>
    {
        uint16_t m_numGroups;

    public:
        LED1D(uint16_t num_leds, int pin = -1)
            : LEDBase<TDriver>(num_leds, pin), m_numGroups(num_leds)
        {
        }

        void update(uint8_t *srcbuff) override
        {
            uint16_t groupSize = this->m_numLeds / m_numGroups;
            uint16_t ledIndex = 0;

            for (uint16_t i = 0; i < m_numGroups; i++)
            {
                for (uint16_t j = 0; j < groupSize; j++)
                {
                    for (uint16_t k = 0; k < this->c_numPxls; k++)
                    {
                        uint16_t srcIndex = i * this->c_numPxls + k;
                        this->m_dstBuffer.get()[ledIndex] = srcbuff[srcIndex];
                        ledIndex++;
                    }
                }
            }
        }

        void setPreset(const JsonDocument &doc) override
        {
            if (!Utils::Json::updateElement<uint16_t>(doc, "num_groups", m_numGroups))
            {
                Utils::Logger::println("[LED_1D] json does not contain 'num_groups'");
            }
        }

        const char* name() const override { return "LED1D"; }

        uint16_t footprint() const override
        {
            return m_numGroups * this->c_numPxls;
        }

        uint16_t footprintFor(const JsonDocument &setting) const override
        {
            uint16_t groups = m_numGroups;
            Utils::Json::updateElement<uint16_t>(setting, "num_groups", groups);
            return groups * this->c_numPxls;
        }

        void toJson(JsonObject &doc) override
        {
            char type[24];
            snprintf(type, sizeof(type), "%s-1D", TDriver::kind);

            doc["uuid"] = this->m_uuid;
            doc["type"] = type;
            doc["hardware_pin"] = this->m_pin;
            doc["num_leds"] = this->m_numLeds;
            doc["num_groups"] = m_numGroups;

            JsonObject desc = doc["description"].to<JsonObject>();
            this->describe(desc);
        }
    };

    // Convenience for native WS2815 strips (runtime pin).
    using WS2815_1D = LED1D<WS2815Driver>;
}
