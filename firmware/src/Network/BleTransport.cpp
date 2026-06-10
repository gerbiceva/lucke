#include "BleTransport.h"
#include <Arduino.h>

namespace Network
{
    void BleTransport::setTxCharacteristic(NimBLECharacteristic* tx)
    {
        m_tx = tx;
    }

    void BleTransport::setMtu(uint16_t mtu)
    {
        m_mtu = mtu;
    }

    void BleTransport::setConnected(bool connected, const std::string& peer)
    {
        m_connected = connected;
        m_peer = peer;
    }

    void BleTransport::send(const std::string& data)
    {
        if (!m_tx || !m_connected)
            return;

        // Frame with a trailing newline so the central can reassemble the
        // chunks into one message (same delimiter the TCP/Serial channels use).
        std::string framed = data;
        framed.push_back('\n');

        // Usable ATT payload is MTU - 3 (ATT opcode + handle). Fall back to the
        // 20-byte default-MTU payload if negotiation has not happened yet.
        uint16_t mtu = m_mtu.load();
        size_t chunk = (mtu > 3) ? static_cast<size_t>(mtu - 3) : 20;

        for (size_t off = 0; off < framed.size(); off += chunk)
        {
            std::string part = framed.substr(off, chunk);
            m_tx->setValue(reinterpret_cast<const uint8_t*>(part.data()), part.size());
            m_tx->notify();
            vTaskDelay(2 / portTICK_PERIOD_MS); // let the host stack drain the queue
        }
    }

    std::string BleTransport::remoteAddress()
    {
        return m_peer.empty() ? "ble" : m_peer;
    }
}
