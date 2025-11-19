#pragma once
#include <cstdint>
#include <vector>

#include <FastLED.h>
#include <ArduinoJson.h>
#include "Traits/Serializable.h"

namespace Traits
{

    class OutputInterface
    {
    protected:
        // std::vector<uint8_t> m_dstBuffer;
        uint8_t* m_dstBuffer;
        uint8_t* m_srcBuffer;
        uint16_t m_size;

    public:
        OutputInterface(uint16_t buffer_size);

        void setSrcBuffer(uint8_t* src_buffer);
    
        std::vector<uint8_t>& getBuffer();
        uint8_t* getBufferPtr();
        uint8_t* getSrcBufferPtr();
        
        // in question
        virtual uint16_t getSize() const;
        virtual void bind() = 0;
        virtual void update() {}
        virtual void setPreset(JsonDocument doc) = 0;

        virtual JsonDocument describe() = 0;
    };

}