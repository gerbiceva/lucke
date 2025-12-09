#pragma once
// #include <atomic>
#include <ArduinoJson.h>
#include <memory>

namespace Utils 
{
class Wifi
{
    Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback);

    static void monitorConnection(void*);

    static std::unique_ptr<Wifi> m_instance;
    const char* m_ssid;
    const char* m_password;
    std::function<void(bool)> m_connection_status_callback;
public:
    static Wifi& initialize(const char* ssid, const char* password, std::function<void(bool)> connection_status_callback);

    static Wifi& instance();

    bool isConnected();
    // bool isConnected() { return connected; }
    // void checkNetwork(void*);

    JsonDocument describe();
};
}