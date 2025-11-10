#pragma once

#include "Outputs.h"

namespace Output
{
    template<uint8_t TPin>
    class Motor : public Outputs
    {

    public:
        Motor(uint8_t num_bytes)
            : Outputs(num_bytes)
        {}

        void bind() override
        {

        }
    };
}