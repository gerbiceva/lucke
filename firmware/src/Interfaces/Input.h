#pragma once
#include <cstdint>
#include <ArduinoJson.h>

namespace Interface
{

struct Input
{
    virtual void begin(uint8_t universe) = 0;
    virtual void stop() = 0;
    virtual void poll(uint8_t* buff) {};
    virtual void toJson(JsonObject& obj) {};
};
}
