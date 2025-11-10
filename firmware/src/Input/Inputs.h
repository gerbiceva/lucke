#pragma once
#include <cstdint>
#include <esp_wifi.h>
#include <WiFi.h>
#include "sACN.h"

#include "Utils/Wifi.h"

namespace Input
{

    enum class InputType
    {
        SACN,
        DMX
    };

    class InputInterface
    {
    protected:
        uint8_t m_dmxBuffer[512] = {0};
        uint8_t m_universe;
        uint8_t m_usage = 0;
        InputType m_type;
    public:
        InputInterface(uint8_t universe)
        : m_universe(universe) {}
        
        virtual ~InputInterface() {}

        uint8_t* getBuffer()
        {
            return m_dmxBuffer;
        }

        virtual void update() = 0;
    };

    struct Sacn : public InputInterface
    {
        WiFiUDP udp;
        Receiver* recv;

        Sacn(uint8_t universe)
            : InputInterface(universe)
        {
            m_type = InputType::SACN;

            if(Utils::Wifi::setup("ledique", "dasenebipovezau"))
            {
                recv = new Receiver(udp);
                recv->begin(universe);
            }
        }

        ~Sacn()
        {
            delete recv;
        }

        void update() override
        {
            recv->dmx(m_dmxBuffer);
            recv->update();
        }
    };
}