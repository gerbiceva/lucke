#include "Wifi.h"
#include "Logger.h"
#include "Core/Engine.h"

// #include <Arduino.h>
#include "WiFi.h"
#include <esp_wifi.h>
#include <platforms/esp/esp_assert.h>

// #include "Handlers/InputHandler.h"

namespace Utils
{
    std::unique_ptr<Wifi> Wifi::m_instance = nullptr;

    // std::atomic<bool> Wifi::connected;

    uint8_t randomInt()
    {
        return static_cast<uint8_t>(random(300));
    }

    void Wifi::monitorConnection(void*) {
         {
            Utils::Logger::println("[TASK] Created 'WIFI check network' task");
            Wifi& instance = Wifi::instance();
            bool is_connected = instance.isConnected();
            // instance.m_connection_status_callback(is_connected);

            while (true) 
            {
                if (!instance.isConnected()) 
                {
                    is_connected = instance.isConnected();
                    instance.m_connection_status_callback(is_connected);
                }

                vTaskDelay(50);
            }
        }
    }


    Wifi::Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback) 
        : m_ssid(ssid), m_password(password), m_connection_status_callback(connection_status_callback) {
            Logger::printf("[WIFI] Connecting to '%s'\n", ssid);
            // uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x10, 0xAF}; // MAC Adress of your device
            // esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
            // if (err != ESP_OK)
            // {
            //     return false;
            // }

            WiFi.setScanMethod(WIFI_FAST_SCAN);
            WiFi.mode(WIFI_STA);
            WiFi.setSleep(false);
            WiFi.begin(m_ssid, m_password);

            xTaskCreate(Wifi::monitorConnection, "Check Wifi", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        }

    Wifi &Wifi::initialize(const char *ssid, const char *password, std::function<void(bool)> connection_status_callback)
    {
        assert(m_instance == nullptr);

        m_instance = std::unique_ptr<Wifi>(new Wifi(ssid, password, connection_status_callback));

        return *m_instance;
    }

    Wifi &Wifi::instance()
    {
        assert(m_instance != nullptr);
        return *m_instance;
    }

    bool Wifi::isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }


    // void Wifi::playIdleAnimation(void*)
    // {
    //     while(true)    //             connected = true;

    //     {
    //         if(Engine::animateWifi != nullptr)
    //         {
    //             Engine::wifiAnimFix->wifiAnimation();
    //             // Engine::animateWifi();
    //         }
    //         vTaskDelay(30);
    //     }
    // }

    // void Wifi::checkNetwork() {
    //     Logger::println("[TASK] Created 'WIFI check network' task");
    //     while (true) 
    //     {
    //         // if not connected
    //         if (WiFi.status() != WL_CONNECTED) 
    //         {
    //             Logger::println("[WIFI] Disconnected, reconnecting...");
    //             Engine::instance().canUpdate(false);

                // TaskHandle_t animation = NULL;
                // xTaskCreate(
                //     Wifi::playIdleAnimation, 		// Task function
                //     "Animation",						// Name of the task (for debugging)
                //     2000,								// Stack size in words
                //     NULL,								// Parameter passed to the task
                //     1,									// Task priority
                //     &animation							// Handle to the task
                // );

                // while not connected
                // uint16_t counter = 0;
                // while (WiFi.status() != WL_CONNECTED) 
                // {
                    // if(counter++ % 10 == 0)
                    // {
                    //     Logger::print(".");
                    // }
                //     vTaskDelay(10);
                // }

                // Logger::println("\n[WIFI] Connected");

                // vTaskDelete(animation);
                // connected = true;
                // Engine::instance().canUpdate(true);
                // Handler::InputHandler::canUpdate(true);
                // Engine::instance().clearSrcBuffers();
    //         }

    //         vTaskDelay(50);
    //     }
    // }

    JsonDocument Wifi::describe()
    {
        JsonDocument doc;
        
        doc["ssid"] = WiFi.SSID();
        doc["rssi"] = WiFi.RSSI();
        doc["local_ip"] = WiFi.localIP();
        doc["connected"] = isConnected();
        return doc;
    }
}