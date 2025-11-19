#include "InputHandler.h"
#include "Input/SACN.h"
#include "Utils/Logger.h"
#include <utility>

namespace Handler
{

    std::unordered_map<uint8_t, Traits::InputInterface*> InputHandler::m_inputs;
    std::vector<std::pair<uint8_t, Traits::InputInterface*>> InputHandler::m_vecInputs;
    
    Traits::InputInterface* InputHandler::interface(uint8_t uni, Traits::InputInterface::InputType type)
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
        {
            Utils::Logger::printf("Error adding interface that is not SACN for uni: %d\n", uni);
            Utils::Logger::println("Adding SACN interface");
            ptr = new Input::Sacn(uni);
        }
    
        m_vecInputs.push_back(std::pair<uint8_t, Traits::InputInterface*>(uni, ptr));
        return ptr;
            
            // m_inputs[uni] = ptr;
        // }
    
        // return m_inputs[uni];
    }
    
    void InputHandler::update()
    {
        for(auto& pair : m_vecInputs)
        {
            pair.second->update();
        }
    }
    
    JsonDocument InputHandler::describe()
    {
        JsonDocument doc;
        doc["inputs"] = JsonDocument();
        JsonArray arr = doc["inputs"].to<JsonArray>();
    
        for(auto& pair : m_vecInputs)
        {
            arr.add(pair.second->describe());
        }
    
        return doc;
    }
}
