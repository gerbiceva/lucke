#pragma once
#include "Interfaces/Sink.h"
#include "Network/BleTransport.h"

namespace Reports
{
    // Pushes reports to a connected BLE central via the NUS TX characteristic.
    // ready() gates on an active connection so the payload is not built (and
    // the shared TX characteristic not spammed) while no client is attached.
    class BleSink : public Interface::Sink
    {
        Network::BleTransport& m_tx;

    public:
        explicit BleSink(Network::BleTransport& tx)
            : m_tx(tx)
        {}

        void publish(const std::string& data) override
        {
            m_tx.send(data);
        }

        bool ready() override
        {
            return m_tx.isConnected();
        }
    };
}
