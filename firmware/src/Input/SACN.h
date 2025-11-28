#pragma once
#include "Traits/Inputs.h"

namespace Input
{
    class Sacn : public Traits::InputInterface
    {
        WiFiUDP udp;
        Receiver* recv;
    public:
        Sacn(uint8_t universe);
        ~Sacn();

        void update() override;

        JsonDocument describe() override;
    };
}