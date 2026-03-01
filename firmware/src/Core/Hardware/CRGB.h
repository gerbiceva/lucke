#pragma once
#include <cstdint>

namespace Core
{

struct CRGB{
    union {
        struct {
            union {
                uint8_t r;
                uint8_t red;
            };

            union {
                uint8_t g;
                uint8_t green;
            };

            union {
                uint8_t b;
                uint8_t blue;
            };
        };

        uint8_t raw[3];
    };

    CRGB()
        : r(0), g(0), b(0)
    {
    }

    CRGB(uint8_t red, uint8_t green, uint8_t blue)
        : r(red), g(green), b(blue)
    {
    }
};

}