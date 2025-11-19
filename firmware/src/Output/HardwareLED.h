#pragma once
#include <cstdint>
#include <FastLED.h>
#include "Traits/Outputs.h"
#include "Handlers/PinHandler.h"
#include "Utils/Logger.h"

namespace Output {

    void updateFastLED();

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
        }

		void bind() override
		{
            if(!PinHandler::available(TPin))
            {
                Utils::Logger::printf("Error adding fastled: pin %d already used\n", TPin);
                return;
            }

            m_cled = &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB*) m_dstBuffer, m_numLeds);
            m_cled->clearLeds();
            updateFastLED();
		};
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