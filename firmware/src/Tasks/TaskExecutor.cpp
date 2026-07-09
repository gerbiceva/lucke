#include "TaskExecutor.h"

namespace Tasks
{

    void Manager::suspendTask(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        }

        it->status = SUSPENDED;
        vTaskSuspend(it->handle);
    }

    void Manager::resumeTask(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        } 

        it->status = RUNNING;
        vTaskResume(it->handle);
    }

    void Manager::stopTask(uint32_t id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        } 

        vTaskDelete(it->handle);
        m_tasks.erase(it);
    }

    bool Manager::isTaskRunning(uint32_t id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return false;
        } 

        return it->status == RUNNING;
    }


    std::list<Manager::TaskRecord>::iterator Manager::find(uint32_t id) 
    {
        return std::find_if(m_tasks.begin(), m_tasks.end(),
            [&](const TaskRecord& r) { return r.id == id; });
    }

    std::list<Manager::TaskRecord>::iterator Manager::find(std::string name) 
    {
        return std::find_if(m_tasks.begin(), m_tasks.end(),
            [&](const TaskRecord& r) { return r.name == name; });
    }
}