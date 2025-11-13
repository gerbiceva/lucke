#pragma once
#include "Traits/Inputs.h"
#include "Input/SACN.h"
#include <unordered_map>
#include <vector>
#include <utility>



class InputHandler
{
    static std::unordered_map<uint8_t, Traits::InputInterface*> m_inputs;
    static std::vector<std::pair<uint8_t, Traits::InputInterface*>> m_vecInputs;
public:
    static Traits::InputInterface* interface(uint8_t uni, Traits::InputInterface::InputType type)
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
        Traits::InputInterface* ptr;

        if(type == Traits::InputInterface::InputType::SACN)
        {
            ptr = new Input::Sacn(uni);
        }
        else
        {}

        m_vecInputs.push_back(std::pair<uint8_t, Traits::InputInterface*>(uni, ptr));
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