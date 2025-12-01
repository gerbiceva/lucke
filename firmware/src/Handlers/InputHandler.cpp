#include "InputHandler.h"
#include "Input/SACN.h"
#include "Utils/Logger.h"
#include <utility>

namespace Handler
{
    using Traits::InputInterface;

    std::vector<InputInterface*> InputHandler::m_inputs;
    TaskHandle_t InputHandler::m_handle;

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
    }

    void InputHandler::canUpdate(bool b)
    {
        static bool m_inited = false;

        if(!m_inited)
        {
            if(b)
            {
                xTaskCreate(InputHandler::updateTask, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, &m_handle);
                m_inited = true;
            }

            return;
        }

        if(b)
        {
            vTaskResume(m_handle);
        }
        else
        {
            vTaskSuspend(m_handle);
        }
    }

    
    void InputHandler::updateTask(void*)
    {
        Utils::Logger::println("[TASK] Created 'DMX Input' task!");

        while(true)
        {
            for(InputInterface* p : m_inputs)
            {
                p->update();
            }
            vTaskDelay(20);
        }
    }

    // void InputHandler::initInputs()
    // {
    //     for(InputInterface* p : m_inputs)
    //     {
    //         p->init();
    //     }
    // }


    void InputHandler::clearSrcBuffers()
    {
        for(InputInterface* p : m_inputs)
        {
            p->clearSrcBuffer();
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
