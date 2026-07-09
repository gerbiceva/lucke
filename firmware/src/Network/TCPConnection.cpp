#include "TCPConnection.h"

namespace Network
{
    TcpConnection::TcpConnection(WiFiClient client)
        : m_client(client)
    {
        // Without this, readStringUntil() blocks for the 1000 ms Arduino
        // stream default after the last byte, adding ~1 s latency per request.
        m_client.setTimeout(50);
    }

    TcpConnection::TcpConnection(const std::string& host, uint16_t port)
    {
        m_client.setTimeout(50);
        m_client.connect(host.c_str(), port);
    }

    bool TcpConnection::isConnected()
    {
        return m_client.connected();
    }

    void TcpConnection::send(const std::string& data)
    {
        if (m_client.connected())
        {
            m_client.print(data.c_str());
            m_client.flush();   // push bytes out before the caller closes the socket
        }
    }

    std::string TcpConnection::readLine()
    {
        if (m_client.available())
        {
            return std::string(m_client.readStringUntil('\n').c_str());
        }
        return "";
    }

    void TcpConnection::flush()
    {
        m_client.flush();
    }

    bool TcpConnection::isAvailable()
    {
        return m_client.available();
    }

    std::string TcpConnection::remoteAddress()
    {
        return std::string(m_client.remoteIP().toString().c_str());
    }

    void TcpConnection::close()
    {
        m_client.stop();
    }
}
