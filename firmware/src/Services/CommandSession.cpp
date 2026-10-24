#include "CommandSession.h"

namespace Services
{
    CommandSession::CommandSession(Interface::Transport& transport, Commands::CommandDispatcher& dispatcher,
                                   Tasks::Manager& tasks, const char* taskName)
        : m_transport(transport), m_dispatcher(dispatcher), m_tasks(tasks), m_taskName(taskName)
    {}

    void CommandSession::start()
    {
        m_tasks.spawnTask(m_taskName, [this]() {
            loop();
        }, 1, 8192);
    }

    void CommandSession::loop()
    {
        while (true)
        {
            if (m_transport.isAvailable())
            {
                std::string line = m_transport.readLine();
                m_dispatcher.dispatch(line, m_transport);
            }
            vTaskDelay(10);
        }
    }
}
