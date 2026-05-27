#pragma once
#include <cstdint>
#include <vector>

#include <FastLED.h>
#include <ArduinoJson.h>

namespace Interface
{

    struct Output
    {

        // void setSrcBuffer(uint8_t* src_buffer);
        // std::vector<uint8_t>& getBuffer();
        // uint8_t* getBufferPtr();
        // uint8_t* getSrcBufferPtr();

        // in question
        virtual uint16_t footprint() const = 0;
        virtual void bind() = 0;
        virtual void update(uint8_t* src) = 0;
        virtual void postupdate() = 0;
        virtual void setPreset(const JsonDocument& doc) = 0;

        virtual void appendBasicInfo(JsonDocument& doc) {};
    };

}
