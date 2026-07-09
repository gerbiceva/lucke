#pragma once
#include <Arduino.h>
#include <cstdint>
#include <list>
#include <mutex>
#include <string>
#include "Utils/Logger.h"


namespace Tasks
{
class Manager {
public:
    enum TaskStatus
    {
        RUNNING,
        SUSPENDED
    };

    Manager() = default;

    template <typename TInvocable>
    uint32_t spawnTask(std::string taskName, TInvocable invocable, uint8_t priority = 1, uint32_t stackSize = 2000) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        uint32_t id = id_generator++;
        TaskRecord rec;
        rec.id = id;
        rec.name = taskName;
        // Tasks are expected to loop forever. If one ever returns, self-delete
        // so the scheduler doesn't run off the end of the task function. The
        // record is intentionally left in place — a task cannot safely erase
        // its own record (that would destroy this very std::function mid-call).
        rec.fn = [invocable]()
        {
            invocable();
            vTaskDelete(NULL);
        };

        Utils::Logger::printf("[TASK] Created new task '%s'\n", taskName.c_str());

        rec.status = RUNNING;
        m_tasks.push_back(rec);

        TaskRecord& stored = m_tasks.back();
        xTaskCreate(
            Manager::trampoline,
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

    bool isTaskRunning(uint32_t id);

private:
    struct TaskRecord {
        uint32_t id;
        std::string name;
        TaskHandle_t handle;
        std::function<void()> fn;
        TaskStatus status;
    };

    std::list<TaskRecord>::iterator find(uint32_t id);
    std::list<TaskRecord>::iterator find(std::string nmae);

    static void trampoline(void* fn) 
    {
        (*reinterpret_cast<std::function<void()>*>(fn))();
    }

    // Guards m_tasks: spawn/suspend/resume/stop are called from several task
    // contexts (e.g. wifi_monitor toggling the fixture/animation tasks).
    std::mutex m_mutex;
    uint32_t id_generator = 0;
    std::list<TaskRecord> m_tasks{};
};
}