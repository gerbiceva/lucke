#pragma once
#include <Arduino.h>
#include "WiFi.h"
#include <esp_wifi.h>

namespace Utils 
{
class Wifi
{
    [[nodiscard]] static uint8_t randomInt();
public:
    [[nodiscard]] static bool setup (const char* ssid, const char* password);
}
}