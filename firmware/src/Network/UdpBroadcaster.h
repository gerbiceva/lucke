#pragma once
#include <WiFiUdp.h>
#include <cstdint>
#include <string>

namespace Net
{
    class UdpBroadcaster
    {
    public:
        void begin(uint16_t port);
        void send(const std::string& data);

    private:
        WiFiUDP m_udp;
        uint16_t m_port = 0;
    };
}
