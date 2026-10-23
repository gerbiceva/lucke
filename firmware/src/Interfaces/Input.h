#pragma once
#include <cstdint>
#include <ArduinoJson.h>
#include "Utils/Dmx.h"

namespace Interface
{

struct Input
{
    virtual ~Input() = default;
    virtual void begin(uint8_t universe) = 0;
    virtual void stop() = 0;
    virtual bool poll(Dmx::UniverseView out) { return false; };
    virtual void toJson(JsonObject& obj) {};
};
}
