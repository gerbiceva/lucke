#include "TcpServer.h"

namespace Net
{
    TcpServer::TcpServer(uint16_t port)
        : m_port(port), m_server(port)
    {}

    void TcpServer::begin()
    {
        m_server.begin();
    }

    void TcpServer::stop()
    {
        m_server.stop();
    }

    void TcpServer::restart()
    {
        stop();
        begin();
    }

    Utils::Optional<TcpConnection> TcpServer::accept()
    {
        WiFiClient client = m_server.accept();
        if (client)
        {
            return TcpConnection(client);
        }
        return Utils::Optional<TcpConnection>();
    }
}
