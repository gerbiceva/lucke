#include "TcpListener.h"
#include "Utils/Logger.h"

namespace
{
    // Drop a client that has sent nothing for this long.
    constexpr TickType_t c_idleTimeout = pdMS_TO_TICKS(10000);

    constexpr Utils::Log s_log{"TCP"};
}

namespace Services
{
    TcpListener::TcpListener(uint16_t port, Commands::CommandDispatcher& dispatcher, Tasks::Manager& tasks)
        : m_server(port), m_dispatcher(dispatcher), m_tasks(tasks)
    {}

    void TcpListener::start()
    {
        m_tasks.spawnTask("tcp_cmd", [this]() {
            loop();
        }, 2, 8192);
    }

    void TcpListener::begin()
    {
        m_server.begin();
    }

    void TcpListener::stop()
    {
        m_server.stop();
    }

    void TcpListener::loop()
    {
        while (true)
        {
            if (auto conn = m_server.accept())
            {
                serve(*conn);
            }
            vTaskDelay(10);
        }
    }

    void TcpListener::serve(Net::TcpConnection& conn)
    {
        const std::string addr = conn.remoteAddress();
        s_log.debug("client %s connected", addr.c_str());

        TickType_t lastActivity = xTaskGetTickCount();
        while (conn.isConnected())
        {
            if (conn.receive() > 0)
            {
                lastActivity = xTaskGetTickCount();
            }

            if (conn.overflowed())
            {
                conn.send("{\"err\":\"request too long\"}");
                break;
            }

            // Handle every complete line already buffered.
            while (conn.isAvailable())
            {
                std::string line = conn.readLine();
                if (!line.empty()) m_dispatcher.dispatch(line, conn);
            }

            if (xTaskGetTickCount() - lastActivity > c_idleTimeout)
            {
                s_log.info("client %s idle, closing", addr.c_str());
                break;
            }
            vTaskDelay(10);
        }
        conn.close();
        s_log.debug("client %s disconnected", addr.c_str());
    }
}
