#pragma once
#include <string>

namespace Traits
{

    struct Deserializable
    {
        virtual void fromJson(std::string json) = 0;
    };
}