#pragma once
#include <Arduino.h>
#include <cstdint>
#include <list>
#include <mutex>
#include <functional>
#include <string>
#include <string_view>
#include "Utils/Logger.h"


namespace Tasks
{
// Opaque task handle. Invalid (0) is never handed out, so it is safe as the
// "not spawned" default.
enum class TaskId : uint32_t
{
    Invalid = 0
};

class Manager {
public:
    enum class TaskStatus
    {
        RUNNING,
        SUSPENDED
    };

    Manager() = default;

    template <typename TInvocable>
    TaskId spawnTask(std::string taskName, TInvocable invocable, uint8_t priority = 1, uint32_t stackSize = 2000) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        TaskId id{id_generator++};
        TaskRecord rec;
        rec.id = id;
        rec.name = std::move(taskName);
        // Tasks are expected to loop forever. If one ever returns, self-delete
        // so the scheduler doesn't run off the end of the task function. The
        // record is intentionally left in place — a task cannot safely erase
        // its own record (that would destroy this very std::function mid-call).
        rec.fn = [invocable = std::move(invocable)]()
        {
            invocable();
            vTaskDelete(NULL);
        };

        rec.status = TaskStatus::RUNNING;
        m_tasks.push_back(std::move(rec));

        TaskRecord& stored = m_tasks.back();
        Utils::Logger::printf("[TASK] Created new task '%s'\n", stored.name.c_str());
        xTaskCreate(
            Manager::trampoline,
            stored.name.c_str(),
            stackSize,
            &stored.fn,
            priority | portPRIVILEGE_BIT,
            &stored.handle
        );

        return stored.id;
    }

    void suspendTask(TaskId id);
    void resumeTask(TaskId id);
    void stopTask(TaskId id);

    bool isTaskRunning(TaskId id);

private:
    struct TaskRecord {
        TaskId id;
        std::string name;
        TaskHandle_t handle;
        std::function<void()> fn;
        TaskStatus status;
    };

    std::list<TaskRecord>::iterator find(TaskId id);
    std::list<TaskRecord>::iterator find(std::string_view name);

    static void trampoline(void* fn) 
    {
        (*reinterpret_cast<std::function<void()>*>(fn))();
    }

    // Guards m_tasks: spawn/suspend/resume/stop are called from several task
    // contexts (e.g. wifi_monitor toggling the fixture/animation tasks).
    std::mutex m_mutex;
    uint32_t id_generator = 1;
    std::list<TaskRecord> m_tasks{};
};
}