#pragma once
#include <cstdint>
#include <vector>
#include <set>

#include <FastLED.h>
#include <ArduinoJson.h>

namespace Output
{

    class FastLEDHelper
    {
    public:
        template<template<uint8_t, EOrder> typename TLedType, uint8_t TPin, EOrder TOrder>
		static CLEDController* make(uint8_t* buffer, uint16_t num_leds)
		{
			return &FastLED.addLeds<TLedType, TPin, TOrder>((CRGB *)buffer, num_leds);
		};

        static void updateFastLED()
        {
            FastLED.show();
        }
    };

    class Outputs
    {
    protected:
        static std::set<uint8_t> m_usedPins;
        // std::vector<uint8_t> m_dstBuffer;
        uint8_t* m_dstBuffer;
        uint8_t* m_srcBuffer;
        uint16_t m_size;

        bool isPinUnused(uint8_t pin) const;
    public:
        Outputs(uint16_t buffer_size);

        void setSrcBuffer(uint8_t* src_buffer);
    
        std::vector<uint8_t>& getBuffer();
        uint8_t* getBufferPtr();
        uint8_t* getSrcBufferPtr();
        
        // in question
        virtual uint16_t getSize() const;
        virtual void bind() = 0;
        virtual void update() {}
        virtual JsonDocument toJson() = 0;
        virtual void setPreset(JsonDocument doc) = 0;

    };

}