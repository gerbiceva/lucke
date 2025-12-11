#pragma once
#include "Input/Button.h"
#include "Traits/Serializable.h"
#include <vector>

namespace Handler
{
    class ButtonManager : public Traits::Serializable
    {
        std::vector<Input::Button> buttons;
        
    public:
        void update();
        void add(Input::Button&& b);
        
        JsonDocument describe() override;        
    };
}

