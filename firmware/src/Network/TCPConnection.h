#pragma once
#include <WiFiClient.h>
#include <cstddef>
#include <optional>
#include <string>
#include "Interfaces/Transport.h"

namespace Net
{
    // One accepted command connection. Requests and replies are single
    // lines of JSON, '\n'-terminated, both on this socket.
    class TcpConnection : public Interface::Transport
    {
    public:
        // Longest request line accepted; a longer one marks the connection
        // overflowed so the listener can drop it.
        static constexpr size_t c_maxLine = 4096;

        TcpConnection() = default;
        explicit TcpConnection(WiFiClient client);

        // Sends data plus the '\n' that ends the reply.
        void send(const std::string& data) override;
        // Next complete line ("" if none), without the '\n' / "\r\n".
        std::string readLine() override;
        // True when a complete line is buffered (reads the socket first).
        bool isAvailable() override;
        std::string remoteAddress() override;
        void close() override;

        // Moves whatever the socket has into the line buffer. Returns the
        // number of bytes read.
        size_t receive();
        [[nodiscard]] bool isConnected();
        [[nodiscard]] bool overflowed() const;

    private:
        [[nodiscard]] std::optional<std::string> takeLine();

        WiFiClient m_client;
        std::string m_rx;
        bool m_overflowed = false;
    };
}
