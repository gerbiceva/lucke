#include "InputHandler.h"
#include "Input/SACN.h"
#include "Utils/Logger.h"
#include <utility>

namespace Handler
{
    using Traits::InputInterface;

    // std::vector<InputInterface*> InputHandler::m_inputs;
    // TaskHandle_t InputHandler::m_handle;

    // std::unordered_map<uint8_t, Traits::InputInterface*> InputHandler::m_inputs;
    // std::vector<std::pair<uint8_t, Traits::InputInterface*>> InputHandler::m_vecInputs;
    
    // InputInterface* InputHandler::find(uint8_t universe)
    // {
    //     for(InputInterface* p : m_inputs)
    //     {
    //         if(p && p->getUniverse() == universe)
    //         {
    //             return p;
    //         }
    //     }
    //
    //     return nullptr;
    // }

    Utils::Optional<InputHandler::Counter*> InputHandler::find(uint8_t universe) {
        if (m_inputs.find(universe) != m_inputs.end()) {
            return &m_inputs[universe];
        }

        return {};
    }

    void InputHandler::iterateInputs(std::function<void(std::shared_ptr<Traits::InputInterface>)> callback) {
        for (auto& it : m_inputs) {
            callback(it.second.interface);
        }
    }


    void InputHandler::unsubscribe(uint8_t universe) {
        auto iface = find(universe);
        if (!iface) {
            return;
        }

        auto& c = iface.value()->count;
        c--;
        if (c == 0) {
            m_inputs.erase(universe);
        }
    }

    std::shared_ptr<Traits::InputInterface> InputHandler::subscribe(uint8_t universe) {
        auto iface = find(universe);
        if (!iface) {
            Counter c;
            c.count = 1;
            c.interface = std::make_shared<Input::Sacn>(universe);
            m_inputs[universe] = std::move(c);
        }
        else {
            iface.value()->count++;
        }

        return m_inputs[universe].interface;
    }

    std::shared_ptr<InputInterface> InputHandler::interface(uint8_t universe, uint8_t old_universe, InputInterface::InputType type)
    {
        unsubscribe(old_universe);
        return subscribe(universe);
    }

    void InputHandler::canUpdate(bool b)
    {
        // static bool m_inited = false;

        // if(!m_inited)
        // {
        //     if(b)
        //     {
        //         xTaskCreate(InputHandler::updateTask, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, &m_handle);
        //         m_inited = true;
        //     }

        //     return;
        // }

        if(b)
        {
            vTaskResume(m_handle);
        }
        else
        {
            vTaskSuspend(m_handle);
        }
    }


    void InputHandler::update()
    {

        while(true)
        {
            // iterateInputs([this](std::shared_ptr<InputInterface> iface) {
            //     iface->update();
            // })
            for (auto& it : m_inputs) {
                it.second.interface->update();
            }
            // for(InputInterface* p : m_inputs)
            // {
            //     p->update();
            // }
            vTaskDelay(20);
        }
    }
    
    void InputHandler::clearSrcBuffers()
    {
        for (auto& it : m_inputs) {
            it.second.interface->clearSrcBuffer();
        }
    }

    void InputHandler::fromJson(std::string json)
    {
        // TODO: does nothing
        JsonDocument doc;
        deserializeJson(doc, json);

        JsonArray arr = doc["inputs"].as<JsonArray>();
        for(JsonDocument a : arr)
        {
            uint8_t universe = static_cast<uint8_t>(a["universe"]);   // extract value
            // interface(universe);
        }
    }

    
    void InputHandler::toJson(JsonObject& doc)
    {
        doc["num_inputs"] = m_inputs.size();
        doc["inputs"] = JsonDocument();
        JsonArray arr = doc["inputs"].to<JsonArray>();

        for (auto& it : m_inputs) {
            JsonObject entry = arr.add<JsonObject>();
            it.second.interface->toJson(entry);
        }
        // for(Traits::InputInterface* i : m_inputs)
        // {
        //     JsonObject entry = arr.add<JsonObject>();
        //     i->toJson(entry);
        // }
    }
}
