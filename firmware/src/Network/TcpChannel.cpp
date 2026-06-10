#include "TcpChannel.h"

namespace Network
{
    TcpChannel::TcpChannel(Commands::CommandDispatcher& dispatcher, uint16_t listenPort, uint16_t responsePort)
        : m_dispatcher(dispatcher), m_server(listenPort), m_responsePort(responsePort)
    {}

    void TcpChannel::begin()
    {
        m_server.begin();
    }

    void TcpChannel::stop()
    {
        m_conn.reset();
        m_server.stop();
    }

    void TcpChannel::poll()
    {
        // No active connection — try to accept one (non-blocking).
        if (!m_conn.has_value())
        {
            auto incoming = m_server.accept();
            if (incoming.has_value())
            {
                m_remoteIp = incoming.value().remoteAddress();
                m_conn = std::move(incoming);
            }
            return;
        }

        TcpConnection& conn = m_conn.value();
        if (!conn.isConnected())
        {
            m_conn.reset();
            return;
        }

        if (conn.isAvailable())
        {
            std::string line = conn.readLine();
            if (!line.empty())
            {
                // Response is delivered on a fresh outbound connection to the
                // client's response port (preserves the existing two-port protocol).
                TcpConnection response(m_remoteIp, m_responsePort);
                m_dispatcher.dispatch(line, response);
                response.close();
            }
        }
    }
}
