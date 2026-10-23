#pragma once
#include <cstdint>
#include "Network/TcpServer.h"
#include "Commands/CommandDispatcher.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // Accepts command connections on the command port, one client at a time.
    // Each reply goes out on a new connection to the client's responsePort.
    class TcpListener
    {
    public:
        TcpListener(uint16_t port, uint16_t responsePort,
                    Commands::CommandDispatcher& dispatcher, Tasks::Manager& tasks);

        // Spawns the accept task. Call once, after everything is constructed.
        void start();

        // Open / close the listening socket (on wifi up / down).
        void begin();
        void stop();

    private:
        void loop();

        Net::TcpServer m_server;
        uint16_t m_responsePort;
        Commands::CommandDispatcher& m_dispatcher;
        Tasks::Manager& m_tasks;
    };
}
