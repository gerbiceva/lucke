#pragma once
#include <ArduinoJson.h>

namespace Traits
{
    struct Deserializable
    {
        virtual JsonDocument toJson() = 0;
        virtual JsonDocument toJsonFull() { return JsonDocument(); }
    };

}