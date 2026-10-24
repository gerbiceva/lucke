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

        while (true) 
        {
            instance.m_connection_status_callback(instance.isConnected());
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
                            instance.m_receive_callback(client.remoteIP().toString().c_str(), client.readString().c_str());
                        }
                    }
                    client.stop();
                }
            }

            vTaskDelay(10);
        }
    }
    // void Wifi::receiveData(void*)
    // {
    //     Utils::Logger::println("[TASK] Created 'WIFI receive data' task");

    //     Wifi& instance = Wifi::instance();

    //     WiFiServer server(8888);
    //     server.begin();

    //     while (true)
    //     {
    //         if (instance.isConnected())
    //         {
    //             WiFiClient client = server.available();  // TCP accept

    //             if (client)
    //             {
    //                 Utils::Logger::println("[TCP] Client connected");

    //                 while (client.connected())
    //                 {
    //                     while (client.available())
    //                     {
    //                         // Read raw TCP bytes
    //                         std::string data = client.readStringUntil('\n').c_str(); 
                            
    //                         if (data.length() > 0)
    //                         {
    //                             instance.m_receive_callback(data.c_str());
    //                         }
    //                     }

    //                     vTaskDelay(10);
    //                 }

    //                 // client.stop();
    //                 // Utils::Logger::println("[TCP] Client disconnected");
    //             }
    //         }

    //         vTaskDelay(10);
    //     }
    // }


    Wifi::Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback, const std::function<void(std::string, std::string)>& receive_callback) 
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

        // initialize(ssid, password, instance().m_connection_status_callback, instance().m_receive_callback);

        return *m_instance;
    }

    Wifi &Wifi::initialize(const char *ssid, const char *password, std::function<void(bool)> connection_status_callback, std::function<void(std::string, std::string)> receive_callback)
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
        xTaskCreate(Wifi::monitorConnection, "Check Wifi", 3000, NULL, 2 | portPRIVILEGE_BIT, NULL);
        xTaskCreate(Wifi::receiveData, "Receive Data", 12000, NULL, 3 | portPRIVILEGE_BIT, NULL);
    }

    bool Wifi::isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }
    
    void Wifi::sendUdpPacket(uint16_t port, std::string data)
    {
        if(isConnected())
        {
            WiFiUDP udp;
            IPAddress ip = WiFi.localIP();
            IPAddress broadcast = IPAddress(ip[0], ip[1], ip[2], 255);

            udp.beginPacket(broadcast, port);
            udp.print(data.c_str());
            udp.endPacket();
        }
    }

    void Wifi::sendTcpPacket(const std::string& host, uint16_t port, std::string data)
    {
        if (!isConnected())
        {
            Utils::Logger::println("WiFi not connected");
            return;
        }

        Utils::Logger::println(("Connecting to: " + host + ":" + std::to_string(port)).c_str());

        WiFiClient client;

        if (!client.connect(host.c_str(), port))
        {
            Utils::Logger::println("TCP connect FAILED");
            Utils::Logger::println(("Error: " + std::to_string(client.getWriteError())).c_str());
            return;
        }

        // Utils::Logger::println("TCP connected");

        size_t sent = client.print(data.c_str());

        // Utils::Logger::println(("Bytes sent: " + std::to_string(sent)).c_str());

        client.stop();
        // Utils::Logger::println("TCP closed");
    }


    // Utils::Optional<std::string>
    Utils::Optional<std::string> Wifi::getIP()
    {
        if(isConnected())
        {
            return std::string(WiFi.localIP().toString().c_str());
        }
        return Utils::Optional<std::string>();
    }
    
    void Wifi::toJson(JsonObject& doc)
    {
        // doc["ssid"] = WiFi.SSID();
        doc["rssi"] = WiFi.RSSI();
        doc["local_ip"] = WiFi.localIP();
        doc["connected"] = isConnected();
    }
}