#include "Wifi.h"
#include "Logger.h"
#include "Engine.h"

#include <Arduino.h>
#include "WiFi.h"
#include <esp_wifi.h>

namespace Utils
{
    std::atomic<bool> Wifi::connected;
    bool Wifi::m_inited = false;

    uint8_t Wifi::randomInt()
    {
        return static_cast<uint8_t>(random(300));
    }
    
    bool Wifi::setup (const char* ssid, const char* password) 
    {
        connected = false;

        Logger::println("Setup Wifi");
        if(m_inited)
        {
            return true;
        }
        // uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x10, 0xAF}; // MAC Adress of your device
        // esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
        // if (err != ESP_OK)
        // {
        //     return false;
        // }

        WiFi.setScanMethod(WIFI_FAST_SCAN);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(ssid, password);
        m_inited = true;
        return true;
    }

    void Wifi::playIdleAnimation(void*)
    {
        while(true)
        {
            if(Engine::wifiAnimFix != nullptr)
            {
                Engine::wifiAnimFix->wifiAnimation();
            }
            vTaskDelay(30);
        }
    }

    void Wifi::checkNetwork(void *) {
    while (true) {
        // if not connected
        if (WiFi.status() != WL_CONNECTED) {
            Logger::println("Disconnected");
            connected = false;

            TaskHandle_t animation = NULL;
            xTaskCreate(
                Wifi::playIdleAnimation, 		// Task function
                "Animation",						// Name of the task (for debugging)
                2000,								// Stack size in words
                NULL,								// Parameter passed to the task
                1,									// Task priority
                &animation							// Handle to the task
            );

            // while not connected
            while (WiFi.status() != WL_CONNECTED) {
                Logger::print(".");
                vTaskDelay(10);
            }

            Logger::println("connected");

            connected = true;
            vTaskDelete(animation);
            // Controller::get().clear();
        }

        vTaskDelay(50);
    }
}
}