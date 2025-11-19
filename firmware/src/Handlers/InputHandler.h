#pragma once
#include "Traits/Inputs.h"
#include <unordered_map>
#include <vector>



class InputHandler
{
    static std::unordered_map<uint8_t, Traits::InputInterface*> m_inputs;
    static std::vector<std::pair<uint8_t, Traits::InputInterface*>> m_vecInputs;
public:
    static Traits::InputInterface* interface(uint8_t uni, Traits::InputInterface::InputType type);
    static void update();
    static JsonDocument describe();
};