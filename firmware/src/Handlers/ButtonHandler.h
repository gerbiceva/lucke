#pragma once
#include <vector>
#include "Input/Button.h"

namespace Handler
{
    class Button
    {
        static std::vector<Button> m_buttons;

    public:
        static void add(Button b)
        {
            m_buttons.push_back(b);
        }

        static void update(void*) {
        while(true) {
            for(auto& b : m_buttons) {
                b.update();
            }

            vTaskDelay(20);
        }
	}
    };
}