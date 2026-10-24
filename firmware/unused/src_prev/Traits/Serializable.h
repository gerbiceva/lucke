#pragma once
#include <string>

namespace Traits
{

    struct Serializable
    {
        virtual void fromJson(std::string json) = 0;
    };
}