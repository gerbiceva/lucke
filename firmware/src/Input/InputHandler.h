#pragma once
#include "Inputs.h"
#include <unordered_map>
#include <vector>
#include <utility>

namespace Input
{

    class Handler
    {
        static std::unordered_map<uint8_t, InputInterface*> m_inputs;
        static std::vector<std::pair<uint8_t, InputInterface*>> m_vecInputs;
    public:
        static InputInterface* interface(uint8_t uni, InputType type)
        {
            bool found = false;
            for(auto& p : m_vecInputs)
            {
                if(p.first == uni)
                {
                    return p.second;
                }
            }
            // if(m_inputs.find(uni) == m_inputs.end())
            // {
            InputInterface* ptr;

            if(type == InputType::SACN)
            {
                ptr = new Sacn(uni);
            }
            else
            {}

            m_vecInputs.push_back(std::pair<uint8_t, InputInterface*>(uni, ptr));
            return ptr;
                
                // m_inputs[uni] = ptr;
            // }

            // return m_inputs[uni];
        }

        static void update()
        {
            for(auto& pair : m_vecInputs)
            {
                pair.second->update();
            }
        }
    };
}