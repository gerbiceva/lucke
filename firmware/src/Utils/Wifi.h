#pragma once
#include <atomic>
#include <ArduinoJson.h>

namespace Utils 
{
class Wifi
{
    static std::atomic<bool> connected;
    static const char* m_ssid;
    static const char* m_password;
    
    [[nodiscard]] static uint8_t randomInt();
    static void playIdleAnimation(void*);
public:
    static bool setup ();
    static bool setup (const char* ssid, const char* password);

    static bool isConnected() { return connected; }
    static void checkNetwork(void*);

    static JsonDocument describe();
};
}