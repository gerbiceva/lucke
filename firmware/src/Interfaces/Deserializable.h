#pragma once
#include <string>

namespace Interface
{
    struct Deserializable
    {
        virtual void fromJson(std::string json) = 0;
    };
}