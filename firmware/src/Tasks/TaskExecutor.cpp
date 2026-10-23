#include "TaskExecutor.h"

namespace Tasks
{

    void Manager::suspendTask(TaskId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        }

        it->status = TaskStatus::SUSPENDED;
        vTaskSuspend(it->handle);
    }

    void Manager::resumeTask(TaskId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        } 

        it->status = TaskStatus::RUNNING;
        vTaskResume(it->handle);
    }

    void Manager::stopTask(TaskId id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        } 

        vTaskDelete(it->handle);
        m_tasks.erase(it);
    }

    bool Manager::isTaskRunning(TaskId id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return false;
        } 

        return it->status == TaskStatus::RUNNING;
    }


    std::list<Manager::TaskRecord>::iterator Manager::find(TaskId id) 
    {
        return std::find_if(m_tasks.begin(), m_tasks.end(),
            [&](const TaskRecord& r) { return r.id == id; });
    }

    std::list<Manager::TaskRecord>::iterator Manager::find(std::string_view name) 
    {
        return std::find_if(m_tasks.begin(), m_tasks.end(),
            [&](const TaskRecord& r) { return r.name == name; });
    }
}