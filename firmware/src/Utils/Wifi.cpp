#include "Wifi.h"

namespace Utils
{
    uint8_t Wifi::randomInt()
    {
        return static_cast<uint8_t>(random(300));
    }
    
    bool Wifi::setup (const char* ssid, const char* password) 
    {
        uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, randomInt() , randomInt()}; // MAC Adress of your device
        esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
        if (err != ESP_OK)
        {
            return false;
            // LOG("Success changing Mac Address\nS");
        }

        WiFi.setScanMethod(WIFI_FAST_SCAN);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(ssid, password);

        return true;
    }
}