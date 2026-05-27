#pragma once
#include <WiFiServer.h>
#include <cstdint>
#include "TCPConnection.h"
#include "Utils/Optional.h"

namespace Network
{
    class TcpServer
    {
    public:
        explicit TcpServer(uint16_t port);

        void begin();
        void stop();
        void restart();
        Utils::Optional<TcpConnection> accept();

    private:
        uint16_t m_port;
        WiFiServer m_server;
    };
}
