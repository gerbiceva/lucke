#pragma once
#include "Input/Button.h"
#include <vector>

namespace Handler
{
    class ButtonManager {
        static std::vector<Input::Button> buttons;
        // std::unordered_map<uint8_t, bool> isPinRegistered;
        
        static void update(void*);
    public:
        static void add(Input::Button&& b);
        static JsonDocument describe();
        
    };
}

