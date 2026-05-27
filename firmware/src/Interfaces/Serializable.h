#pragma once
#include <ArduinoJson.h>

namespace Interface
{

    struct Serializable
    {
        virtual void toJson(JsonObject& obj) = 0;
        virtual JsonDocument toJsonDoc() { return JsonDocument(); };
        // virtual void fromJson(std::string json) = 0;
    };
}