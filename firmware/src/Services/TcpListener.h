#pragma once
#include <cstdint>
#include "Network/TcpServer.h"
#include "Network/TCPConnection.h"
#include "Commands/CommandDispatcher.h"
#include "Tasks/TaskExecutor.h"

namespace Services
{
    // Accepts command connections on the command port, one client at a
    // time. Each request line gets its reply line on the same connection;
    // the client may send several before closing. A client that goes quiet
    // is dropped after an idle timeout so others can connect.
    class TcpListener
    {
    public:
        TcpListener(uint16_t port, Commands::CommandDispatcher& dispatcher, Tasks::Manager& tasks);

        // Spawns the accept task. Call once, after everything is constructed.
        void start();

        // Open / close the listening socket (on wifi up / down).
        void begin();
        void stop();

    private:
        void loop();
        void serve(Net::TcpConnection& conn);

        Net::TcpServer m_server;
        Commands::CommandDispatcher& m_dispatcher;
        Tasks::Manager& m_tasks;
    };
}
