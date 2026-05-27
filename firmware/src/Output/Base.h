#pragma once
#include "Interfaces/Output.h"
#include "Interfaces/Serializable.h"

namespace Output
{
    class Base : public Interface::Output, public Interface::Serializable
    {
        static uint16_t s_uuid;
    protected:
        uint16_t m_uuid;
        uint16_t m_size;

    public:
        Base(uint16_t size);

        uint16_t footprint() const override;
        JsonDocument toJsonDoc() override {return JsonDocument();};
    };
}