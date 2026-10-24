#pragma once
#include <WiFiServer.h>
#include <cstdint>
#include "TCPConnection.h"
#include <optional>

namespace Net
{
    class TcpServer
    {
    public:
        explicit TcpServer(uint16_t port);

        void begin();
        void stop();
        std::optional<TcpConnection> accept();

    private:
        WiFiServer m_server;
    };
}
