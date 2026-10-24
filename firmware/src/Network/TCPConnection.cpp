#include "TCPConnection.h"
#include <array>

namespace
{
    // WiFiClient uses its timeout for writes too, so it must cover sending
    // a large reply (describe) over a weak link.
    constexpr uint32_t c_writeTimeoutMs = 2000;
}

namespace Net
{
    TcpConnection::TcpConnection(WiFiClient client)
        : m_client(client)
    {
        m_client.setTimeout(c_writeTimeoutMs);
        // Each reply is one write; send it now instead of waiting on Nagle.
        m_client.setNoDelay(true);
    }

    bool TcpConnection::isConnected()
    {
        return m_client.connected();
    }

    void TcpConnection::send(const std::string& data)
    {
        if (!m_client.connected()) return;

        std::string line;
        line.reserve(data.size() + 1);
        line += data;
        line += '\n';
        m_client.write(reinterpret_cast<const uint8_t*>(line.data()), line.size());
    }

    size_t TcpConnection::receive()
    {
        size_t total = 0;
        std::array<uint8_t, 256> chunk;
        while (m_client.available() > 0)
        {
            int n = m_client.read(chunk.data(), chunk.size());
            if (n <= 0) break;
            total += n;

            if (m_rx.size() + n > c_maxLine)
            {
                m_overflowed = true;
                m_rx.clear();
                break;
            }
            m_rx.append(reinterpret_cast<const char*>(chunk.data()), n);
        }
        return total;
    }

    std::optional<std::string> TcpConnection::takeLine()
    {
        size_t end = m_rx.find('\n');
        if (end == std::string::npos) return std::nullopt;

        std::string line = m_rx.substr(0, end);
        m_rx.erase(0, end + 1);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        return line;
    }

    std::string TcpConnection::readLine()
    {
        return takeLine().value_or("");
    }

    bool TcpConnection::isAvailable()
    {
        receive();
        return m_rx.find('\n') != std::string::npos;
    }

    bool TcpConnection::overflowed() const
    {
        return m_overflowed;
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
