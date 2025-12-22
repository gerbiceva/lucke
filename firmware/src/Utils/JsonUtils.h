#pragma once
#include <ArduinoJson.h>
#include <string>

namespace Utils::Json
{
    template<typename T>
    T getElement(JsonDocument& doc, std::string name, T alternative)
    {
        if(doc[name].is<T>())
        {
            return doc[name];
        }

        return alternative;
    }

    template<typename T>
    void updateElement(JsonDocument& doc, std::string name, T& element)
    {
        if(doc[name].is<T>())
        {
            element = doc[name];
        }
    }
};