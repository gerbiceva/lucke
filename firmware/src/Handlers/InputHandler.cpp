#include "InputHandler.h"
#include "Input/SacnSource.h"
#include "Utils/Logger.h"

namespace Handler
{
    bool InputHandler::match(uint8_t uni)
    {
        return m_sources.find(uni) != m_sources.end();
    }
    
    std::shared_ptr<SyncBuffer> InputHandler::subscribe(uint8_t uni)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(match(uni))
        {
            m_refcount[uni]++;
        }
        else
        {
            auto src = std::make_shared<Input::SacnSource>(uni);
            if (m_ready) src->begin(uni);
            m_sources[uni] = src;
            m_buffers[uni] = std::make_shared<SyncBuffer>();
            m_refcount[uni] = 1;
        }

        return m_buffers[uni];
    }

    void InputHandler::unsubscribe(uint8_t uni)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!match(uni))
        {
            return;
        }
        if(--m_refcount[uni] == 0)
        {
            m_sources[uni]->stop();
            m_sources.erase(uni);
            m_buffers.erase(uni);
            m_refcount.erase(uni);
        }
    }

    void InputHandler::update()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint8_t tmp[512];
        for(auto& pair : m_sources)
        {
            if (pair.second->poll(tmp))
            {
                m_buffers[pair.first]->write(tmp);
            }
        }
    }

    void InputHandler::reconnect()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ready = true;
        for (auto& pair : m_sources)
        {
            pair.second->stop();
            pair.second->begin(pair.first);
        }
    }


    void InputHandler::toJson(JsonObject& obj)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        obj["num_inputs"] = m_sources.size();
        JsonArray arr = obj["inputs"].to<JsonArray>();
        for (auto& pair : m_sources)
        {
            JsonObject entry = arr.add<JsonObject>();
            pair.second->toJson(entry);
        }
    }

}