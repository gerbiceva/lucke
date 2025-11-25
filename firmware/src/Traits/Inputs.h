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
        uint8_t m_usage = 0;
        InputType m_type;

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
