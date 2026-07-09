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

        // Logical output type, driver-independent ("LED1D", "Grid", ...).
        virtual const char* name() const { return "Output"; }
        // Hardware pin this output drives, or -1 when not applicable (a grid
        // with several strips, or a non-pin output).
        virtual int hardwarePin() const { return -1; }

        // Writes a human-facing summary of this output. Base contributes the
        // fields every output has (name, footprint, hardware pin); subclasses
        // extend with their own dimensions (num_leds, width/height, ...).
        virtual void describe(JsonObject& desc)
        {
            desc["name"] = name();
            desc["footprint"] = footprint();
            desc["hardware_pin"] = hardwarePin();
        }
    };
}