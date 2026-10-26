#pragma once
// #include <atomic>
#include <memory>
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
};
}