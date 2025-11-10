#pragma once
#include <cstdint>
#include <FastLED.h>
#include "Outputs.h"


namespace Output {

    template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class HardwareLED : public Outputs
    {
    protected:
        uint16_t m_numLeds;
        uint8_t m_numPxls = 3U;

        CLEDController* m_cled;
    public:

        HardwareLED(uint16_t num_leds)
            : Outputs(num_leds * m_numPxls), m_numLeds(num_leds)
        {
        }

		void bind() override
		{
            if(!isPinUnused(TPin))
            {
                return;
            }

            // memset(m_dstBuffer, 0, m_size);

            m_cled = FastLEDHelper::make<TLedType, TPin, TOrder>(m_dstBuffer, m_numLeds);
            m_cled->clearLeds();
            FastLEDHelper::updateFastLED();
		};


        // JsonDocument toJson() override
        // {
        //     JsonDocument doc;


        //     return doc;
        // }
    };

    template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class HardwareLED1D : public HardwareLED<TLedType, TPin, TOrder>
    {
        uint16_t m_numGroups = 1;
    public:
        HardwareLED1D(uint16_t num_leds)
            : HardwareLED<TLedType, TPin, TOrder>(num_leds),
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

        void setPreset(JsonDocument doc) override
        {
            m_numGroups = doc["num_groups"];
        }

        uint16_t getSize() const override
        {
            return m_numGroups * this->m_numPxls;
        }

        JsonDocument toJson() override
        {
            JsonDocument doc;
            doc["type"] = "FastLED-1D";
            doc["num_groups"] = m_numGroups;

            return doc;
        }
    };
}

//     template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
//     class HardwareLED2D : public HardwareLED<TLedType, TPin, TOrder>
//     {
//     uint8_t m_width;
//     uint8_t m_height;
//     uint8_t m_column_width;
//     uint8_t m_row_height;
//     uint8_t m_num_columns;
//     uint8_t m_num_rows;

//     // std::vector<Preset2D> m_presets;

// 	std::unordered_map<uint32_t, std::vector<uint32_t>> m_grid_hash;

//     std::vector<uint32_t>& getGridIndexes(uint8_t x, uint8_t y);
// public:
//     Fixture2D(std::string name, std::string type, uint8_t width, uint8_t height);

//     void update() override;
//     void setPreset(uint8_t newPreset) override;

//     [[nodiscard]] JsonDocument presetsToJson() const override;

    // };