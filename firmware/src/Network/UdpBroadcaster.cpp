#include "UdpBroadcaster.h"
#include <WiFi.h>

namespace Net
{
    void UdpBroadcaster::begin(uint16_t port)
    {
        m_port = port;
        m_udp.begin(port);
    }

    void UdpBroadcaster::send(const std::string& data)
    {
        if (WiFi.status() != WL_CONNECTED) return;

        IPAddress ip = WiFi.localIP();
        IPAddress broadcast(ip[0], ip[1], ip[2], 255);

        m_udp.beginPacket(broadcast, m_port);
        m_udp.print(data.c_str());
        m_udp.endPacket();
    }
}
