#pragma once
// #include <atomic>
#include <memory>
#include <functional>
#include "Utils/Optional.h"
#include "Interfaces/Serializable.h"


namespace Network 
{
class Wifi : public Interface::Serializable
{
    // Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback, const std::function<void(std::string, std::string)>& receive_callback);

public:
    void connect(const std::string& ssid, const std::string& pass);
    void reconnect(const std::string& ssid, const std::string& pass);
    void disconnect();

    bool isConnected() const;
    std::string localIP() const;
    int rssi() const;
    // static Wifi& initialize(const char* ssid, const char* password, std::function<void(bool)> connection_status_callback, std::function<void(std::string, std::string)> receive_callback);
    // static Wifi& reinitialize(const char* ssid, const char* password);
    // static Wifi& instance();

    // static void monitorConnection(void*);
    // static void receiveData(void*);
    // static void startTasks();
    
    // bool isConnected();
    // Utils::Optional<std::string> getIP();

    // void sendUdpPacket(uint16_t port, std::string data);
    // void sendTcpPacket(const std::string& hostip, uint16_t port, std::string data);

    void toJson(JsonObject& doc) override;

private:
    std::string m_ssid;
    std::string m_password;

    std::function<void(bool)> m_connection_status_callback;
    // std::function<void(std::string, std::string)> m_receive_callback;
};
}