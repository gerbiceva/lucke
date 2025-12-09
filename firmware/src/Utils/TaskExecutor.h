#pragma once
#include <Arduino.h>
#include <cstdint>
#include "Utils/Logger.h"
#include <map>

namespace Utils
{
class TaskExecutor {
public:
    TaskExecutor() = default;

    template <typename TInvocable>
    uint32_t spawnTask(TInvocable invocable) 
    {
        uint32_t id = id_generator++;

        m_tasks.insert({id, {nullptr, [this, id, invocable]() 
        { 
            Utils::Logger::printf("Created task: %d\n", id);
            invocable();
            //m_tasks.erase(it);
            Utils::Logger::printf("Destroyed task: %d\n", id);
            vTaskDelete(nullptr);
        }}});

        xTaskCreate(TaskExecutor::trampoline, std::to_string(id).c_str(), 2000, &m_tasks[id].second, 1 | portPRIVILEGE_BIT, &m_tasks[id].first);

        return id;
    }

    void stopTask(uint32_t id) {
        auto it = m_tasks.find(id);

        if (it == m_tasks.end()) {
            return;
        } 

        vTaskDelete(it->second.first);
        m_tasks.erase(id);
    }

private:
    static void trampoline(void* fn) 
    {
        (*reinterpret_cast<std::function<void()>*>(fn))();
    }

    uint32_t id_generator = 0;
    std::map<uint32_t, std::pair<TaskHandle_t, std::function<void()>>> m_tasks{};
};
}