#include "BleChannel.h"
#include "Utils/Logger.h"

namespace Network
{
    // Nordic UART Service UUIDs — recognised by nRF Connect and generic BLE
    // UART terminal apps, so the channel works with off-the-shelf clients.
    static const char* NUS_SERVICE = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    static const char* NUS_RX      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; // central writes
    static const char* NUS_TX      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // device notifies

    BleChannel::BleChannel(Commands::CommandDispatcher& dispatcher, const std::string& deviceName)
        : m_dispatcher(dispatcher), m_deviceName(deviceName)
    {}

    void BleChannel::begin()
    {
        if (m_started) return;

        NimBLEDevice::init(m_deviceName);
        NimBLEDevice::setMTU(247); // request a larger MTU; the central may negotiate down

        m_server = NimBLEDevice::createServer();
        m_server->setCallbacks(new ServerCallbacks(*this));

        NimBLEService* service = m_server->createService(NUS_SERVICE);

        m_rx = service->createCharacteristic(
            NUS_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
        m_rx->setCallbacks(new RxCallbacks(*this));

        m_tx = service->createCharacteristic(NUS_TX, NIMBLE_PROPERTY::NOTIFY);
        m_transport.setTxCharacteristic(m_tx);

        service->start();

        NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
        adv->addServiceUUID(NUS_SERVICE);
        adv->setScanResponse(true);
        adv->start();

        m_started = true;
        Utils::Logger::printf("[BLE] advertising as '%s'\n", m_deviceName.c_str());
    }

    void BleChannel::stop()
    {
        if (!m_started) return;
        NimBLEDevice::stopAdvertising();
        NimBLEDevice::deinit(true);
        m_server = nullptr;
        m_rx = nullptr;
        m_tx = nullptr;
        m_started = false;
    }

    void BleChannel::enqueue(const std::string& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_assembly += data;

        size_t pos;
        while ((pos = m_assembly.find('\n')) != std::string::npos)
        {
            std::string line = m_assembly.substr(0, pos);
            m_assembly.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (!line.empty())
                m_lines.push_back(std::move(line));
        }

        // Drop a runaway unterminated payload rather than grow without bound.
        if (m_assembly.size() > 2048)
            m_assembly.clear();
    }

    void BleChannel::poll()
    {
        for (;;)
        {
            std::string line;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_lines.empty())
                    break;
                line = std::move(m_lines.front());
                m_lines.pop_front();
            }
            m_dispatcher.dispatch(line, m_transport);
        }
    }

    // ---------------------------------------------------------------- callbacks

    void BleChannel::ServerCallbacks::onConnect(NimBLEServer* server)
    {
        m_owner.m_transport.setConnected(true);
        Utils::Logger::println("[BLE] client connected");
    }

    void BleChannel::ServerCallbacks::onDisconnect(NimBLEServer* server)
    {
        m_owner.m_transport.setConnected(false);
        Utils::Logger::println("[BLE] client disconnected, re-advertising");
        NimBLEDevice::startAdvertising();
    }

    void BleChannel::ServerCallbacks::onMTUChange(uint16_t mtu, ble_gap_conn_desc* desc)
    {
        m_owner.m_transport.setMtu(mtu);
    }

    void BleChannel::RxCallbacks::onWrite(NimBLECharacteristic* characteristic)
    {
        std::string value = characteristic->getValue();
        if (!value.empty())
            m_owner.enqueue(value);
    }
}
