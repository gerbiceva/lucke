#pragma once
// #include <atomic>
#include <memory>
#include <functional>
#include <string>
#include "Interfaces/Serializable.h"


namespace Net
{
struct WifiInfo
{
    bool connected = false;
    std::string ip;
    int rssi = 0;

    void toJson(JsonObject& obj)
    {
        obj["rssi"] = rssi;
        obj["local_ip"] = ip;
        obj["connected"] = connected;
    }
};

class Wifi : public Interface::Serializable
{
    // Wifi(const char *ssid, const char *password, const std::function<void(bool)>& connection_status_callback, const std::function<void(std::string, std::string)>& receive_callback);

public:
    void connect(const std::string& ssid, const std::string& pass);
    void reconnect(const std::string& ssid, const std::string& pass);
    void disconnect();

    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] std::string localIP() const;
    [[nodiscard]] int rssi() const;
    [[nodiscard]] WifiInfo info() const;

    void toJson(JsonObject& doc) override;

private:
    std::string m_ssid;
    std::string m_password;

    std::function<void(bool)> m_connection_status_callback;
    // std::function<void(std::string, std::string)> m_receive_callback;
};
}