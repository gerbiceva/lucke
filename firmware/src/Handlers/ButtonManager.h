#pragma once
#include "Input/Button.h"
#include "Traits/Deserializable.h"
#include <vector>

namespace Handler
{
    class ButtonManager : public Traits::Deserializable
    {
        std::vector<Input::Button> buttons;
        
    public:
        void update();
        void add(Input::Button&& b);
        
        JsonDocument toJson() override;        
    };
}

