#pragma once
#include <atomic>
#include <string>
#include <NimBLECharacteristic.h>
#include "Interfaces/Transport.h"

namespace Network
{
    // Response side of the BLE command channel. send() writes the reply back to
    // the connected central via notifications on the TX characteristic, chunked
    // to the negotiated ATT MTU. The read side is unused during dispatch (the
    // BleChannel hands the already-read line to the dispatcher), so readLine()/
    // isAvailable() are inert.
    class BleTransport : public Interface::Transport
    {
    public:
        BleTransport() = default;

        void setTxCharacteristic(NimBLECharacteristic* tx);
        void setMtu(uint16_t mtu);
        void setConnected(bool connected, const std::string& peer = "");
        bool isConnected() const { return m_connected.load(); }

        void send(const std::string& data) override;
        std::string readLine() override { return ""; }
        bool isAvailable() override { return false; }
        std::string remoteAddress() override;
        void close() override {}

    private:
        NimBLECharacteristic* m_tx = nullptr;
        std::atomic<uint16_t> m_mtu{23};
        std::atomic<bool> m_connected{false};
        std::string m_peer;
    };
}
