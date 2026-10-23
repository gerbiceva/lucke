
#pragma once

namespace Output
{
    // Sends one pixel buffer to LED hardware. The buffer belongs to the
    // output and is only borrowed here: it must outlive the driver and never
    // be reallocated.
    struct Driver
    {
        virtual ~Driver() = default;
        virtual void show() = 0;
        [[nodiscard]] virtual const char* kind() const = 0;
        [[nodiscard]] virtual int pin() const = 0;
    };
}