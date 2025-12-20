#pragma once
#include <ArduinoJson.h>

namespace Traits
{
    struct Deserializable
    {
        virtual void toJson(JsonObject& doc) = 0;
        virtual void toJsonFull(JsonObject& doc) {  }
    };

}