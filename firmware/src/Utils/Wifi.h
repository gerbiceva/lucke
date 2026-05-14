#pragma once
// #include <atomic>
#include <memory>
#include <functional>
#include "Utils/Optional.h"
#include "Traits/Deserializable.h"


namespace Utils 
{
class Wifi : public Traits::Deserializable
{
    Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback, const std::function<void(std::string)>& receive_callback);

public:
    static Wifi& initialize(const char* ssid, const char* password, std::function<void(bool)> connection_status_callback, std::function<void(std::string)> receive_callback);
    static Wifi& reinitialize(const char* ssid, const char* password);
    static Wifi& instance();

    static void monitorConnection(void*);
    static void receiveData(void*);
    static void startTasks();
    
    bool isConnected();
    Utils::Optional<std::string> getIP();

    void sendUdpPacket(uint16_t port, std::string data);

    void toJson(JsonObject& doc) override;

private:
    static std::unique_ptr<Wifi> m_instance;
    const char* m_ssid;
    const char* m_password;

    std::function<void(bool)> m_connection_status_callback;
    std::function<void(std::string)> m_receive_callback;
};
}