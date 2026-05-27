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

// #pragma once
// #include <cstdarg>
// #include <cstdio>
// #include <string>

// namespace Utils
// {
//     class String
//     {
//     public:
//         static std::string concat(const char* format, ...)
//         {
//             va_list args;
//             va_start(args, format);

//             // First pass: determine required size
//             va_list args_copy;
//             va_copy(args_copy, args);
//             int size = std::vsnprintf(nullptr, 0, format, args_copy);
//             va_end(args_copy);

//             if (size < 0)
//             {
//                 va_end(args);
//                 return {};
//             }

//             // Allocate buffer (+1 for null terminator)
//             std::string result(size, '\0');
//             std::vsnprintf(result.data(), size + 1, format, args);

//             va_end(args);
//             return result;
//         }
//     };
// }