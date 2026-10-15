// #include "BleTransport.h"
// #include <BLEUtils.h>
// #include <BLE2902.h>
// #include <esp_mac.h>
// #include <cstdio>
// #include "Utils/Logger.h"
//
// namespace Net
// {
//     // Nordic UART Service — widely recognized convention, so off-the-shelf
//     // "BLE UART" client apps/libraries work against this out of the box.
//     static const char* SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
//     static const char* RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; // write (central -> device)
//     static const char* TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // notify (device -> central)
//
//     class BleTransport::ServerCallbacks : public BLEServerCallbacks
//     {
//     public:
//         void onConnect(BLEServer* server) override
//         {
//             Utils::Logger::println("[BLE] Central connected");
//         }
//
//         void onDisconnect(BLEServer* server) override
//         {
//             Utils::Logger::println("[BLE] Central disconnected, resuming advertising");
//             server->getAdvertising()->start();
//         }
//     };
//
//     class BleTransport::RxCallbacks : public BLECharacteristicCallbacks
//     {
//     public:
//         explicit RxCallbacks(BleTransport* owner) : m_owner(owner) {}
//
//         void onWrite(BLECharacteristic* characteristic) override
//         {
//             String value = characteristic->getValue();
//             m_owner->onLineChunk(std::string(value.c_str(), value.length()));
//         }
//
//     private:
//         BleTransport* m_owner;
//     };
//
//     void BleTransport::begin(const std::string& deviceNamePrefix)
//     {
//         // Suffix with the last two MAC bytes so multiple devices are
//         // distinguishable in a scan list — no wifi connection required.
//         uint8_t mac[6];
//         esp_read_mac(mac, ESP_MAC_WIFI_STA);
//         char suffix[6];
//         snprintf(suffix, sizeof(suffix), "%02X%02X", mac[4], mac[5]);
//         std::string deviceName = deviceNamePrefix + "-" + suffix;
//
//         BLEDevice::init(deviceName.c_str());
//
//         m_server = BLEDevice::createServer();
//         m_server->setCallbacks(new ServerCallbacks());
//
//         BLEService* service = m_server->createService(SERVICE_UUID);
//
//         m_rxChar = service->createCharacteristic(
//             RX_CHAR_UUID,
//             BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
//         m_rxChar->setCallbacks(new RxCallbacks(this));
//
//         m_txChar = service->createCharacteristic(
//             TX_CHAR_UUID,
//             BLECharacteristic::PROPERTY_NOTIFY);
//         m_txChar->addDescriptor(new BLE2902());
//
//         service->start();
//
//         BLEAdvertising* advertising = BLEDevice::getAdvertising();
//         advertising->addServiceUUID(SERVICE_UUID);
//         advertising->setScanResponse(true);
//         advertising->start();
//
//         Utils::Logger::printf("[BLE] Advertising as '%s'\n", deviceName.c_str());
//     }
//
//     void BleTransport::onLineChunk(const std::string& chunk)
//     {
//         std::lock_guard<std::mutex> lock(m_mutex);
//         m_rxAccum += chunk;
//
//         size_t pos;
//         while ((pos = m_rxAccum.find('\n')) != std::string::npos)
//         {
//             m_lines.push_back(m_rxAccum.substr(0, pos));
//             m_rxAccum.erase(0, pos + 1);
//         }
//     }
//
//     void BleTransport::send(const std::string& data)
//     {
//         if (!m_txChar) return;
//
//         std::string framed = data;
//         framed += '\n';
//
//         // BLE_ATT_MTU is negotiated per-connection; stay well under the
//         // legacy default (23) minus the 3-byte ATT header so this works even
//         // before/without MTU negotiation completing.
//         constexpr size_t CHUNK_SIZE = 20;
//
//         for (size_t offset = 0; offset < framed.size(); offset += CHUNK_SIZE)
//         {
//             std::string chunk = framed.substr(offset, CHUNK_SIZE);
//             m_txChar->setValue(reinterpret_cast<uint8_t*>(chunk.data()), chunk.size());
//             m_txChar->notify();
//             vTaskDelay(2);
//         }
//     }
//
//     std::string BleTransport::readLine()
//     {
//         std::lock_guard<std::mutex> lock(m_mutex);
//         if (m_lines.empty()) return "";
//
//         std::string line = m_lines.front();
//         m_lines.pop_front();
//         return line;
//     }
//
//     bool BleTransport::isAvailable()
//     {
//         std::lock_guard<std::mutex> lock(m_mutex);
//         return !m_lines.empty();
//     }
//
//     std::string BleTransport::remoteAddress()
//     {
//         return "ble";
//     }
//
//     void BleTransport::close()
//     {
//         // The BLE connection is managed by the central; nothing to close here.
//     }
// }
