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
    T getElement(const JsonDocument& doc, std::string name, T alternative)
    {
        if(doc[name].is<T>())
        {
            return doc[name];
        }

        return alternative;
    }

    template<typename T>
    bool updateElement(JsonDocument& doc, std::string name, T& element)
    {
        if(doc[name].is<T>())
        {
            element = doc[name];
            return true;
        }

        return false;
    }

    template<typename T>
    bool updateElement(const JsonDocument& doc, std::string name, T& element)
    {
        if(doc[name].is<T>())
        {
            element = doc[name];
            return true;
        }

        return false;
    }
};