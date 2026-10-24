#include "InputHandler.h"
#include "Input/SACN.h"
#include "Utils/Logger.h"
#include <utility>

namespace Handler
{
    using Traits::InputInterface;

    std::shared_ptr<Traits::InputInterface> InputHandler::find(uint8_t universe) {
        auto it = m_inputs.find(universe);

        if (it != m_inputs.end()) {
            return it->second;
        }

        return nullptr;
    }

    void InputHandler::iterateInputs(std::function<void(std::shared_ptr<Traits::InputInterface>)> callback) {
        for (auto& it : m_inputs) 
        {
            callback(it.second);
        }
    }

    std::shared_ptr<Traits::InputInterface> InputHandler::subscribe(uint8_t universe) {
        auto iface = find(universe);
        if (iface == nullptr) 
        {
            m_inputs[universe] = std::make_shared<Input::Sacn>(universe);
            Utils::Logger::printf("Subscr:%p \n", m_inputs[universe]->getBuffer());
        }

        return m_inputs[universe];
    }


    void InputHandler::unsubscribe(uint8_t universe) {
        m_inputs.erase(universe);
        // auto it = m_inputs.find(universe);
        // if (it == m_inputs.end()) return;

        // use_count == 2: one ref in the map, one in the calling fixture (which is about to be reassigned)
        // If it's > 2, other fixtures still use this universe — don't erase
        // if (it->second.use_count() <= 2) {
            // m_inputs.erase(it);
        // }s
    }

    std::shared_ptr<Traits::InputInterface> InputHandler::newInterface(uint8_t universe, Traits::InputInterface::InputType type) {
        return subscribe(universe);
    }

    std::shared_ptr<InputInterface> InputHandler::interface(uint8_t universe, uint8_t old_universe, InputInterface::InputType type)
    {
        for(auto& it : m_inputs)
        {
            Utils::Logger::printf("%p \n", it.second->getBuffer());
        }
        
        unsubscribe(old_universe);
        return subscribe(universe);
    }

    void InputHandler::update()
    {
        while(true)
        {
            for (auto& it : m_inputs) {
                it.second->update();
            }
            vTaskDelay(20);
        }
    }
    
    void InputHandler::clearSrcBuffers()
    {
        for (auto& it : m_inputs) 
        {
            it.second->clearSrcBuffer();
        }
    }

    void InputHandler::reconnectInputs()
    {
        for (auto& it : m_inputs) 
        {
            it.second->reinit();
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
            it.second->toJson(entry);
        }
        // for(Traits::InputInterface* i : m_inputs)
        // {
        //     JsonObject entry = arr.add<JsonObject>();
        //     i->toJson(entry);
        // }
    }
}
