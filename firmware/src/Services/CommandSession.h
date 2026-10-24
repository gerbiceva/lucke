#pragma once
#include "Interfaces/Transport.h"
#include "Commands/CommandDispatcher.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // Runs the line-based command protocol over one always-open transport
    // (serial, BLE): read a line, dispatch it, reply on the same transport.
    class CommandSession
    {
    public:
        CommandSession(Interface::Transport& transport, Commands::CommandDispatcher& dispatcher,
                       Tasks::Manager& tasks, const char* taskName);

        // Spawns the session task. Call once, after everything is constructed.
        void start();

    private:
        void loop();

        Interface::Transport& m_transport;
        Commands::CommandDispatcher& m_dispatcher;
        Tasks::Manager& m_tasks;
        const char* m_taskName;
    };
}
