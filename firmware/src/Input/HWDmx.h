#pragma once
#include "Traits/Inputs.h"

namespace Input
{
    template<uint8_t RXpin, uint8_t TXpin>
    class HWDmx : public Traits::InputInterface
    {
         HWDmx() : Traits::InputInterface()
         {
            m_type = InputType::DMX;
         }
        ~HWDmx() {}

        
        void update() override {}
        // virtual void setUniverse(uint8_t) override {}

        JsonDocument describe() { return JsonDocument(); }
    };
}