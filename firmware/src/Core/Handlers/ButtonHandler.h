#pragma once
#include <vector>
#include "Input/Button.h"
#include "Traits/Deserializable.h"

namespace Handler
{
    class Button : public Traits::Deserializable
    {
        std::vector<Input::Button> buttons;
        
    public:
        void update();
        void add(Input::Button&& b);
        
        void toJson(JsonObject& doc) override;        
    };
}

