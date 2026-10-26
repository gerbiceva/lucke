#pragma once
#include <Arduino.h>
#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include "Utils/Logger.h"


namespace Tasks
{
// Opaque task handle. Invalid (0) is never handed out, so it is safe as the
// "not spawned" default.
enum class TaskId : uint32_t
{
    Invalid = 0
};

// Spawns the long-running FreeRTOS tasks and keeps their records alive.
// Tasks are never stopped from outside: deleting a task can leave a lock it
// holds taken forever. A task that should pause checks a flag instead (see
// RenderLoop, Telemetry).
class Manager {
public:
    Manager() = default;

    // Returns TaskId::Invalid if the task couldn't be created.
    template <typename TInvocable>
    TaskId spawnTask(std::string taskName, TInvocable invocable, uint8_t priority = 1, uint32_t stackSize = 2000)
    {
        std::scoped_lock lock(m_mutex);

        TaskRecord& rec = m_tasks.emplace_back();
        rec.id = TaskId{id_generator++};
        rec.name = std::move(taskName);
        // Tasks are expected to loop forever. If one ever returns, self-delete
        // so the scheduler doesn't run off the end of the task function. The
        // record is intentionally left in place — a task cannot safely erase
        // its own record (that would destroy this very function mid-call).
        rec.fn = [invocable = std::move(invocable)]()
        {
            invocable();
            vTaskDelete(nullptr);
        };

        // The record lives in a std::list, so &rec.fn stays valid for the
        // task's lifetime.
        BaseType_t created = xTaskCreate(
            Manager::trampoline,
            rec.name.c_str(),
            stackSize,
            &rec.fn,
            priority | portPRIVILEGE_BIT,
            &rec.handle
        );
        if (created != pdPASS)
        {
            s_log.error("failed to create task '%s' (stack %lu)", rec.name.c_str(), static_cast<unsigned long>(stackSize));
            m_tasks.pop_back();
            return TaskId::Invalid;
        }

        s_log.debug("created task '%s'", rec.name.c_str());
        return rec.id;
    }

    // Debug: unused stack per task. Assumes no task has returned.
    void logStackHeadroom()
    {
        std::scoped_lock lock(m_mutex);
        std::string line;
        for (const TaskRecord& rec : m_tasks)
        {
            if (!line.empty()) line += ", ";
            line += rec.name + " " + std::to_string(uxTaskGetStackHighWaterMark(rec.handle));
        }
        s_log.debug("stack free: %s", line.c_str());
    }

private:
    static constexpr Utils::Log s_log{"TASK"};

    struct TaskRecord {
        TaskId id = TaskId::Invalid;
        std::string name;
        TaskHandle_t handle = nullptr;
        std::move_only_function<void()> fn;
    };

    static void trampoline(void* fn)
    {
        (*reinterpret_cast<std::move_only_function<void()>*>(fn))();
    }

    // Guards m_tasks: tasks are spawned from setup() and from other tasks.
    std::mutex m_mutex;
    uint32_t id_generator = 1;
    std::list<TaskRecord> m_tasks{};
};
}
