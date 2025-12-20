#pragma once
#include "HardwareLED.h"
#include <unordered_map>
#include <vector>

namespace Output
{
    template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
    class HardwareLED2D : public HardwareLED<TLedType, TPin, TOrder>
    {
        uint8_t m_width;
        uint8_t m_height;
        uint8_t m_column_width;
        uint8_t m_row_height;
        uint8_t m_num_columns;
        uint8_t m_num_rows;

        std::unordered_map<uint32_t, std::vector<uint32_t>> m_grid_hash;

        std::vector<uint32_t>& getGridIndexes(uint8_t x, uint8_t y)
        {
            uint32_t hnum = x * 7741 + y * 7757;
                
            if(m_grid_hash.find(hnum) != m_grid_hash.end()) 
            {
                return m_grid_hash[hnum];
            }

            for(uint32_t yi = y * m_row_height; yi < (y + 1) * m_row_height; yi++) 
            {
                for(uint32_t xi = x * m_column_width; xi < (x + 1) * m_column_width; xi++) 
                {
                    m_grid_hash[hnum].push_back(xi + yi * m_width);
                }
            }

            return m_grid_hash[hnum];
        }
    public:
        HardwareLED2D(uint8_t width, uint8_t height)
            : m_width(width), m_height(height), m_column_width(2), m_row_height(2),
              HardwareLED<TLedType, TPin, TOrder>(width * height)
        {
            m_num_columns = (m_width / m_column_width);
            m_num_rows = (m_height / m_row_height);
            m_grid_hash.clear();
        }

        void update() override
        {
            for(uint8_t y = 0; y < m_num_rows; y++) 
            {
                for(uint8_t x = 0; x < m_num_columns; x++) 
                {
                    const auto& indexes = getGridIndexes(x,y);
                    uint16_t dmxIndex = (x * this->m_numPxls) + (y * this->m_numPxls) * m_num_columns;
                    for(auto index : indexes) 
                    {
                        for (uint16_t k = 0; k < this->m_numPxls; k++) 
                        {
                            // LOGF("led[%d] = dmx [%d]\n", (index * lamp->numPxls + k), (dmxIndex + k));
                            this->m_dstBuffer[index * this->m_numPxls + k] = this->m_srcBuffer[dmxIndex + k]; 
                        }
                    }
                }
            }
        }

        void setPreset(const JsonDocument& doc) override
        {
            m_row_height = doc["row_height"];
            m_column_width = doc["column_width"];

            m_num_columns = (m_width / m_column_width);
            m_num_rows = (m_height / m_row_height);
            m_grid_hash.clear();
        }

        void toJson(JsonObject& doc) override
        {
            doc["type"] = "FastLED-2D";
            doc["hardware_pin"] = TPin;
            doc["num_leds"] = this->m_numLeds;
            doc["width"] = m_width;
            doc["height"] = m_height;
            doc["column_width"] = m_column_width;
            doc["row_height"] = m_row_height;
            doc["num_columns"] = m_num_columns;
            doc["num_rows"] = m_num_rows;
        }

    };
}