#pragma once
#include <string>

namespace Interface
{
    struct Deserializable
    {
        virtual ~Deserializable() = default;
        virtual void fromJson(const std::string& json) = 0;
    };
}