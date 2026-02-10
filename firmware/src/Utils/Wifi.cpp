#include "Wifi.h"
#include "Logger.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

namespace Utils
{
    std::unique_ptr<Wifi> Wifi::m_instance = nullptr;

    uint8_t randomInt()
    {
        return static_cast<uint8_t>(random(300));
    }

    void Wifi::monitorConnection(void*) 
    {
        Utils::Logger::println("[TASK] Created 'WIFI check network' task");
        Wifi& instance = Wifi::instance();
        bool is_connected = instance.isConnected();

        while (true) 
        {
            is_connected = instance.isConnected();
            if(!is_connected)
            {
                instance.m_connection_status_callback(is_connected);
            }

            vTaskDelay(50);
        }
    }

    void Wifi::receiveData(void*) 
    {
        Utils::Logger::println("[TASK] Created 'WIFI receive data' task");
        Wifi& instance = Wifi::instance();
        bool is_connected = instance.isConnected();

        WiFiServer server(8888);
        server.begin();
        while (true) 
        {
            if (instance.isConnected()) 
            {
                WiFiClient client = server.accept();
                if (client) 
                {
                    while (client.connected()) 
                    {
                        if (client.available()) 
                        {
                            instance.m_receive_callback(client.readString().c_str());
                        }
                    }
                    client.stop();
                }
            }

            vTaskDelay(50);
        }
    }


    Wifi::Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback, const std::function<void(std::string)>& receive_callback) 
        : m_ssid(ssid), m_password(password), m_connection_status_callback(connection_status_callback), m_receive_callback(receive_callback) {
            Logger::printf("[WIFI] Connecting to '%s'\n", m_ssid);
            // uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x10, randomInt()}; // MAC Adress of your device
            // esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
            // if (err != ESP_OK)
            // {
            //     Logger::printf("[WIFI] Unable to set mac address '%s'\n", m_ssid);
            //     return;
            // }

            WiFi.setScanMethod(WIFI_FAST_SCAN);
            WiFi.mode(WIFI_STA);
            WiFi.setSleep(false);
            WiFi.begin(m_ssid, m_password);

            // xTaskCreate(Wifi::monitorConnection, "Check Wifi", 3000, NULL, 1 | portPRIVILEGE_BIT, NULL);
            // xTaskCreate(Wifi::receiveData, "Receive Data", 4000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        }

    Wifi& Wifi::reinitialize(const char* ssid, const char* password)
    {
        Logger::printf("[WIFI] Reinitializing WiFi\n");

        WiFi.disconnect(true);   // true = erase old config
        WiFi.mode(WIFI_OFF);
        delay(100);

        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(ssid, password);

        return *m_instance;
    }

    Wifi &Wifi::initialize(const char *ssid, const char *password, std::function<void(bool)> connection_status_callback, std::function<void(std::string)> receive_callback)
    {
        assert(m_instance == nullptr);
        m_instance = std::unique_ptr<Wifi>(new Wifi(ssid, password, connection_status_callback, receive_callback));
        return *m_instance;
    }

    Wifi &Wifi::instance()
    {
        assert(m_instance != nullptr);
        return *m_instance;
    }

    void Wifi::startTasks()
    {
        xTaskCreate(Wifi::monitorConnection, "Check Wifi", 3000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        xTaskCreate(Wifi::receiveData, "Receive Data", 12000, NULL, 1 | portPRIVILEGE_BIT, NULL);
    }

    bool Wifi::isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }
    
    void Wifi::sendUdpPacket(uint16_t port, const std::string& data)
    {
        if(isConnected())
        {
            WiFiUDP udp;
            udp.beginPacket(WiFi.broadcastIP(), port);
            udp.print(data.c_str());
            udp.endPacket();
        }
    }
    
    void Wifi::toJson(JsonObject& doc)
    {
        // doc["ssid"] = WiFi.SSID();
        doc["rssi"] = WiFi.RSSI();
        doc["local_ip"] = WiFi.localIP();
        doc["connected"] = isConnected();
    }
}