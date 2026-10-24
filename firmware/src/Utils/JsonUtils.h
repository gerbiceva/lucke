#pragma once
#include <ArduinoJson.h>
#include <string_view>

namespace Utils::Json
{
    // Both take JsonVariantConst, so they accept a JsonDocument, JsonObject or
    // JsonVariant, const or not.

    template<typename T>
    T getElement(JsonVariantConst doc, std::string_view name, T alternative)
    {
        JsonVariantConst value = doc[name];
        return value.is<T>() ? value.as<T>() : alternative;
    }

    template<typename T>
    bool updateElement(JsonVariantConst doc, std::string_view name, T& element)
    {
        JsonVariantConst value = doc[name];
        if (!value.is<T>())
        {
            return false;
        }

        element = value.as<T>();
        return true;
    }
}
