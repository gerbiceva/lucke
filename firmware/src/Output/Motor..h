#pragma once

#include "Traits/Outputs.h"

namespace Output
{
    template<uint8_t TPin>
    class Motor : public Traits::OutputInterface
    {

    public:
        Motor(uint8_t num_bytes)
            : Traits::OutputInterface(num_bytes)
        {}

        void bind() override
        {

        }
    };
}