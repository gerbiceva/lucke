#include "TcpListener.h"
#include "Network/TCPConnection.h"

namespace Services
{
    TcpListener::TcpListener(uint16_t port, uint16_t responsePort,
                             Commands::CommandDispatcher& dispatcher, Tasks::Manager& tasks)
        : m_server(port), m_responsePort(responsePort), m_dispatcher(dispatcher), m_tasks(tasks)
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
            auto conn = m_server.accept();
            if (conn.has_value())
            {
                auto& tc = conn.value();
                std::string remoteIp = tc.remoteAddress();
                while (tc.isConnected())
                {
                    if (tc.isAvailable())
                    {
                        std::string line = tc.readLine();
                        Net::TcpConnection response(remoteIp, m_responsePort);
                        m_dispatcher.dispatch(line, response);
                        response.close();
                    }
                    vTaskDelay(10);
                }
            }
            vTaskDelay(10);
        }
    }
}
