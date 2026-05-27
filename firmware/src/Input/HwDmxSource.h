#pragma once
#include "Interfaces/Input.h"

namespace Input
{
    class HwDmxSource : public Interface::Input
    {
    public:
        void begin(uint8_t universe) override {}
        void stop() override {}
        void poll(uint8_t* buff) override {}

    private:
        uint8_t m_universe = 0;
    };
}
