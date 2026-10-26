#pragma once
#include "Interfaces/Output.h"

namespace Output
{
    class Base : public Interface::Output
    {
        static uint16_t s_uuid;
    protected:
        uint16_t m_uuid;
        uint16_t m_size;

    public:
        Base(uint16_t size);

        uint16_t footprint() const override;
    };
}