#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <NimBLEDevice.h>

#include "Interfaces/CommandChannel.h"
#include "Commands/CommandDispatcher.h"
#include "Network/BleTransport.h"

namespace Network
{
    // BLE command channel exposing a Nordic UART Service (NUS): the central
    // writes newline-delimited command lines to the RX characteristic and
    // receives responses as notifications on the TX characteristic. Writes
    // arrive in the NimBLE host task and are queued; poll() (run from the
    // command task) drains complete lines into the dispatcher.
    class BleChannel : public Interface::CommandChannel
    {
    public:
        BleChannel(Commands::CommandDispatcher& dispatcher, const std::string& deviceName);

        void begin() override;
        void stop() override;
        void poll() override;

        // Outbound side, used by Reports::BleSink to broadcast periodic reports
        // to the connected central over the same NUS TX characteristic.
        BleTransport& transport() { return m_transport; }

    private:
        class ServerCallbacks : public NimBLEServerCallbacks
        {
        public:
            explicit ServerCallbacks(BleChannel& owner) : m_owner(owner) {}
            void onConnect(NimBLEServer* server) override;
            void onDisconnect(NimBLEServer* server) override;
            void onMTUChange(uint16_t mtu, ble_gap_conn_desc* desc) override;

        private:
            BleChannel& m_owner;
        };

        class RxCallbacks : public NimBLECharacteristicCallbacks
        {
        public:
            explicit RxCallbacks(BleChannel& owner) : m_owner(owner) {}
            void onWrite(NimBLECharacteristic* characteristic) override;

        private:
            BleChannel& m_owner;
        };

        void enqueue(const std::string& data);

        Commands::CommandDispatcher& m_dispatcher;
        std::string m_deviceName;

        NimBLEServer* m_server = nullptr;
        NimBLECharacteristic* m_rx = nullptr;
        NimBLECharacteristic* m_tx = nullptr;
        BleTransport m_transport;

        std::string m_assembly;           // partial line accumulator
        std::deque<std::string> m_lines;  // complete lines awaiting dispatch
        std::mutex m_mutex;

        bool m_started = false;
    };
}
