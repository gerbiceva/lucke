// #pragma once
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <deque>
// #include <mutex>
// #include <string>
// #include "Interfaces/Transport.h"
//
// namespace Net
// {
//     // BLE GATT transport using the Nordic UART Service (NUS) UUIDs, so any
//     // generic "BLE UART" client app can talk to it. Central writes newline-
//     // delimited command lines to RX; responses are notified on TX, chunked to
//     // the negotiated MTU and newline-terminated so multi-chunk messages can be
//     // reassembled on the other end — same line-based contract as Serial/TCP.
//     class BleTransport : public Interface::Transport
//     {
//     public:
//         void begin(const std::string& deviceName);
//
//         void send(const std::string& data) override;
//         std::string readLine() override;
//         bool isAvailable() override;
//         std::string remoteAddress() override;
//         void close() override;
//
//         // Called by the RX characteristic's write callback.
//         void onLineChunk(const std::string& chunk);
//
//     private:
//         class ServerCallbacks;
//         class RxCallbacks;
//
//         BLEServer* m_server = nullptr;
//         BLECharacteristic* m_rxChar = nullptr;
//         BLECharacteristic* m_txChar = nullptr;
//
//         std::mutex m_mutex;
//         std::string m_rxAccum;
//         std::deque<std::string> m_lines;
//     };
// }
