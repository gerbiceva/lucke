#pragma once
#include <Arduino.h>
#include <cstdint>
#include "Utils/Logger.h"
#include <list>

namespace Utils
{
class TaskExecutor {
public:
    TaskExecutor() = default;

    template <typename TInvocable>
    TaskHandle_t& spawnTask(std::string taskName, TInvocable invocable, uint8_t priority = 1, uint32_t stackSize = 2000) 
    {
        uint32_t id = id_generator++;
        TaskRecord rec;
        rec.id = id;
        rec.fn = [this, id, invocable]() 
        { 
            Utils::Logger::printf("Created task: %d\n", id);
            invocable();
            // m_tasks.erase(it);
            Utils::Logger::printf("Destroyed task: %d\n", id);
            // vTaskDelete(nullptr);
            this->stopTask(id);
        };

        m_tasks.push_back(rec);

        TaskRecord& stored = m_tasks.back();
        xTaskCreate(
            TaskExecutor::trampoline,
            taskName.c_str(),
            stackSize,
            &stored.fn,
            priority | portPRIVILEGE_BIT,
            &stored.handle
        );

        // m_tasks.insert({id, {nullptr, [this, id, invocable]() 
        // { 
        //     Utils::Logger::printf("Created task: %d\n", id);
        //     invocable();
        //     //m_tasks.erase(it);
        //     Utils::Logger::printf("Destroyed task: %d\n", id);
        //     vTaskDelete(nullptr);
        // }}});

        // xTaskCreate(TaskExecutor::trampoline, std::to_string(id).c_str(), 2000, &m_tasks[id].second, 1 | portPRIVILEGE_BIT, &m_tasks[id].first);

        return stored.handle;
    }

    void stopTask(uint32_t id) {
        auto it = find(id);

        if (it == m_tasks.end()) {
            return;
        } 

        vTaskDelete(it->handle);
        m_tasks.erase(it);
    }

private:
    struct TaskRecord {
        uint32_t id;
        TaskHandle_t handle;
        std::function<void()> fn;
    };

    std::list<TaskRecord>::iterator find(uint32_t id) 
    {
        return std::find_if(m_tasks.begin(), m_tasks.end(),
            [&](const TaskRecord& r) { return r.id == id; });
    }

    static void trampoline(void* fn) 
    {
        (*reinterpret_cast<std::function<void()>*>(fn))();
    }

    uint32_t id_generator = 0;
    std::list<TaskRecord> m_tasks{};
};
}