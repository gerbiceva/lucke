#pragma once
#include "Traits/Inputs.h"
#include <unordered_map>
#include <vector>


namespace Handler
{
    class InputHandler
    {
        static std::vector<Traits::InputInterface*> m_inputs;
        // static std::unordered_map<uint8_t, Traits::InputInterface*> m_inputs;
        // static std::vector<std::pair<uint8_t, Traits::InputInterface*>> m_vecInputs;

        static Traits::InputInterface* find(uint8_t universe);
    public:
        static Traits::InputInterface* interface(uint8_t universe, Traits::InputInterface::InputType type);
        static void update();
        static JsonDocument describe();
    };
}
