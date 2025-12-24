#include "TaskExecutor.h"

namespace Utils
{

    void TaskExecutor::suspendTask(uint32_t id)
    {
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        }

        it->status = SUSPENDED;
        vTaskSuspend(it->handle);
    }

    void TaskExecutor::resumeTask(uint32_t id)
    {
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        } 

        it->status = RUNNING;
        vTaskResume(it->handle);
    }

    void TaskExecutor::stopTask(uint32_t id) {
        auto it = find(id);

        if (it == m_tasks.end()) 
        {
            return;
        } 

        vTaskDelete(it->handle);
        m_tasks.erase(it);
    }


    std::list<TaskExecutor::TaskRecord>::iterator TaskExecutor::find(uint32_t id) 
    {
        return std::find_if(m_tasks.begin(), m_tasks.end(),
            [&](const TaskRecord& r) { return r.id == id; });
    }
}