#pragma once
// #include <atomic>
#include <memory>
#include "Traits/Serializable.h"


namespace Utils 
{
class Wifi : public Traits::Serializable
{
    Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback);

public:
    static Wifi& initialize(const char* ssid, const char* password, std::function<void(bool)> connection_status_callback);
    
    static Wifi& instance();
    
    static void monitorConnection(void*);
    bool isConnected();

    JsonDocument describe() override;

private:
    static std::unique_ptr<Wifi> m_instance;
    const char* m_ssid;
    const char* m_password;

    std::function<void(bool)> m_connection_status_callback;
};
}