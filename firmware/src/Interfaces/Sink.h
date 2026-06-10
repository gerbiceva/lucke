#pragma once
#include <string>

namespace Interface
{
    // One-way outbound counterpart to Transport: a destination that periodic
    // reports / broadcasts are pushed to (UDP, Serial, BLE notify, ...). The
    // Reports::Publisher drives every Sink the same way, so a new output target
    // is added without touching the report scheduling logic.
    struct Sink
    {
        virtual void publish(const std::string& data) = 0;
        // Gate checked before a feed's payload is built — skip producing when
        // the sink cannot deliver (e.g. wifi down, no BLE client connected).
        virtual bool ready() { return true; }
        virtual ~Sink() = default;
    };
}
