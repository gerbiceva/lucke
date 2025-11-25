#include "InputHandler.h"
#include "Input/SACN.h"
#include "Utils/Logger.h"
#include <utility>

namespace Handler
{
    using Traits::InputInterface;

    std::vector<InputInterface*> InputHandler::m_inputs;

    // std::unordered_map<uint8_t, Traits::InputInterface*> InputHandler::m_inputs;
    // std::vector<std::pair<uint8_t, Traits::InputInterface*>> InputHandler::m_vecInputs;
    
    InputInterface* InputHandler::find(uint8_t universe)
    {
        for(InputInterface* p : m_inputs)
        {
            if(p && p->getUniverse() == universe)
            {
                return p;
            }
        }

        return nullptr;
    }

    InputInterface* InputHandler::interface(uint8_t universe, InputInterface::InputType type)
    {
        // if(m_inputs.find(uni) == m_inputs.end())
        // {
        InputInterface* ptr = find(universe);
        if(!ptr)
        {
            ptr = new Input::Sacn(universe);
            m_inputs.push_back(ptr);
        }
        
        return ptr; 
    
        // if(type == Traits::InputInterface::InputType::SACN)
        // {
        //     ptr = new Input::Sacn(uni);
        // }
        // else
        // {
        //     Utils::Logger::printf("Error adding interface that is not SACN for uni: %d\n", uni);
        //     Utils::Logger::println("Adding SACN interface");
        //     ptr = new Input::Sacn(uni);
        // }
    
        // m_vecInputs.push_back(std::pair<uint8_t, Traits::InputInterface*>(uni, ptr));
        // return ptr;
            
            // m_inputs[uni] = ptr;
        // }
    
        // return m_inputs[uni];
    }
    
    void InputHandler::update()
    {
        for(InputInterface* p : m_inputs)
        {
            p->update();
        }
    }
    
    JsonDocument InputHandler::describe()
    {
        JsonDocument doc;
        doc["inputs"] = JsonDocument();
        JsonArray arr = doc["inputs"].to<JsonArray>();
    
        for(InputInterface* p : m_inputs)
        {
            arr.add(p->describe());
        }
    
        return doc;
    }
}
