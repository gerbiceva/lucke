#pragma once
#include <atomic>

namespace Utils 
{
class Wifi
{
    [[nodiscard]] static uint8_t randomInt();
    static bool m_inited;
    static std::atomic<bool> connected;
public:
    static bool setup (const char* ssid, const char* password);

    static void checkNetwork(void*);
};
}