#pragma once
#include "TestHardwareLED.h"
#include "Utils/JsonUtils.h"

namespace Output
{
    class TestHardwareLED1D : public TestHardwareLED
    {
        uint16_t m_numGroups = 1;
    public:
        TestHardwareLED1D(uint16_t num_leds, uint8_t pin)
            : TestHardwareLED(num_leds, pin),
              m_numGroups(num_leds)
        {
        }

        void update() override
        {
            uint16_t groupSize = this->m_numLeds / m_numGroups;
            uint16_t ledIndex = 0;

            for (uint16_t i = 0; i < m_numGroups; i++) {
                for (uint16_t j = 0; j < groupSize; j++) {
                    for (uint16_t k = 0; k < this->m_numPxls; k++) {
            //             // check if in bounds
                        uint16_t dmxBufferIndex = i * this->m_numPxls + k;
                        this->m_dstBuffer[ledIndex] = this->m_srcBuffer[dmxBufferIndex];
                        ledIndex++;
                    }
                }
            }
        }

        void setPreset(const JsonDocument& doc) override
        {
            if(!Utils::Json::updateElement<uint16_t>(doc, "num_groups", m_numGroups))
            // if(doc["num_groups"].is<uint16_t>())
            {
                // m_numGroups = doc["num_groups"];
                Utils::Logger::println("[HLED_1D] json does not contain 'num_groups'");
            }
            // else
            // {
            // }
        }

        uint16_t getSize() const override
        {
            return m_numGroups * this->m_numPxls;
        }

        void toJson(JsonObject& doc) override
        {
            doc["type"] = "FastLED-1D";
            // doc["hardware_pin"] = TPin;
            doc["num_leds"] = this->m_numLeds;
            doc["num_groups"] = m_numGroups;
        }
    };
}