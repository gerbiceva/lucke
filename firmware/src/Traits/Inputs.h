#pragma once
#include <cstdint>
#include <esp_wifi.h>
#include <WiFi.h>
#include "sACN.h"

#include "Utils/Wifi.h"
#include <ArduinoJson.h>
#include "Traits/Serializable.h"

namespace Traits
{

    class InputInterface
    {
        static uint8_t s_ID;
    public:
        enum class InputType
        {
            None,
            SACN,
            DMX
        };
    protected:
        uint8_t* m_dmxBuffer;
        uint8_t m_universe;
        InputType m_type;
        // uint8_t m_ID;

        std::string typeToString() const;
    public:
        InputInterface(uint8_t universe);
        
        virtual ~InputInterface();
        uint8_t* getBuffer();
        uint8_t getUniverse() const;

        virtual void setUniverse(uint8_t universe) { m_universe = universe; }
        virtual void update() = 0;
        virtual JsonDocument describe() = 0;
    };
}
