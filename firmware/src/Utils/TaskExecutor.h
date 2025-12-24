#pragma once
#include <Arduino.h>
#include <cstdint>
#include <list>
#include <string>
#include "Utils/Logger.h"


namespace Utils
{
class TaskExecutor {
public:
    enum TaskStatus
    {
        RUNNING,
        SUSPENDED
    };

    TaskExecutor() = default;

    template <typename TInvocable>
    uint32_t spawnTask(std::string taskName, TInvocable invocable, uint8_t priority = 1, uint32_t stackSize = 2000) 
    {
        uint32_t id = id_generator++;
        TaskRecord rec;
        rec.id = id;
        rec.fn = [this, id, invocable]() 
        { 
            invocable();
            this->stopTask(id);
        };

        Utils::Logger::printf("[TASK] Created new task '%s'\n", taskName.c_str());

        rec.status = RUNNING;
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

        return stored.id;
    }

    void suspendTask(uint32_t id);
    void resumeTask(uint32_t id);
    void stopTask(uint32_t id);

private:
    struct TaskRecord {
        uint32_t id;
        TaskHandle_t handle;
        std::function<void()> fn;
        TaskStatus status;
    };

    std::list<TaskRecord>::iterator find(uint32_t id);

    static void trampoline(void* fn) 
    {
        (*reinterpret_cast<std::function<void()>*>(fn))();
    }

    uint32_t id_generator = 0;
    std::list<TaskRecord> m_tasks{};
};
}