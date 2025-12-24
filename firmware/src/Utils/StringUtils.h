#pragma once
#include <sstream>
#include <string>

namespace Utils
{
    class String
    {
        static void concat(std::stringstream& ss)
        {
        }
    
        template<typename T, typename... Args>
        static void concat(std::stringstream& ss, T&& value, Args&&... args)
        {
            ss << std::forward<T>(value);
            concat(ss, std::forward<Args>(args)...);
        }
    public:
        template<typename... Args>
        static std::string concat(Args&&... args)
        {
            std::stringstream ss;
            concat(ss, std::forward<Args>(args)...);
            return ss.str();
        }
    };
}