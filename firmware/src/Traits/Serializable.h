#pragma once
#include <ArduinoJson.h>

namespace Traits
{
    struct Serializable
    {
        virtual JsonDocument describe() = 0;
    };

}