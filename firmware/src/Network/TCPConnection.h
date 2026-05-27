#pragma once
#include <WiFiClient.h>
#include "Interfaces/Transport.h"

namespace Network
{
    class TcpConnection : public Interface::Transport
    {
    public:
        TcpConnection() = default;
        explicit TcpConnection(WiFiClient client);
        TcpConnection(const std::string& host, uint16_t port);

        void send(const std::string& data) override;
        std::string readLine() override;
        void flush();
        bool isAvailable() override;
        bool isConnected();
        std::string remoteAddress() override;
        void close() override;

    private:
        WiFiClient m_client;
    };
}