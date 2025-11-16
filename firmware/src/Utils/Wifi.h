#pragma once
#include <atomic>

namespace Utils 
{
class Wifi
{
    static bool m_inited;
    static std::atomic<bool> connected;
    
    [[nodiscard]] static uint8_t randomInt();
    static void playIdleAnimation(void*);
public:
    static bool setup (const char* ssid, const char* password);

    static bool isConnected() { return connected; }
    static void checkNetwork(void*);
};
}