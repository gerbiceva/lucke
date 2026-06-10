#pragma once
#include <cstdint>
#include "Interfaces/Sink.h"
#include "Network/Wifi.h"
#include "Network/UdpBroadcaster.h"

namespace Reports
{
    // Broadcasts reports over UDP (heartbeat, wireless status). Only delivers
    // while wifi is up; begin() must be called once the network is available
    // (see Engine::onWifiStatus).
    class UdpSink : public Interface::Sink
    {
        Network::Wifi& m_wifi;
        Network::UdpBroadcaster m_udp;
        uint16_t m_port;

    public:
        UdpSink(Network::Wifi& wifi, uint16_t port)
            : m_wifi(wifi), m_port(port)
        {}

        void begin()
        {
            m_udp.begin(m_port);
        }

        void publish(const std::string& data) override
        {
            m_udp.send(data);
        }

        bool ready() override
        {
            return m_wifi.isConnected();
        }
    };
}
