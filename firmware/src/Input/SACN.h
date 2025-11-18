#pragma once
#include "Traits/Inputs.h"

namespace Input
{
    class Sacn : public Traits::InputInterface
    {
        WiFiUDP udp;
        Receiver* recv;
    public:
        Sacn(uint8_t universe)
            : InputInterface(universe)
        {
            m_type = InputType::SACN;
            recv = new Receiver(udp);
            recv->begin(universe);
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