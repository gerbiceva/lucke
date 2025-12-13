#pragma once
// #include <atomic>
#include <memory>
#include "Traits/Deserializable.h"


namespace Utils 
{
class Wifi : public Traits::Deserializable
{
    Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback);

public:
    static Wifi& initialize(const char* ssid, const char* password, std::function<void(bool)> connection_status_callback);
    static Wifi& instance();
    
    static void monitorConnection(void*);
    bool isConnected();

    void sendUdpPacket(uint16_t port, const std::string& data);

    JsonDocument toJson() override;

private:
    static std::unique_ptr<Wifi> m_instance;
    const char* m_ssid;
    const char* m_password;

    std::function<void(bool)> m_connection_status_callback;
};
}