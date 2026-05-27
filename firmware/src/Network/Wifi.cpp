#include "Wifi.h"
#include "Utils/Logger.h"

#include <WiFi.h>
#include <esp_wifi.h>

namespace Network
{
    void Wifi::connect(const std::string& ssid, const std::string& pass)
    {
        m_ssid = ssid;
        m_password = pass;

        Utils::Logger::printf("[WIFI] Connecting to '%s'\n", m_ssid.c_str());

        WiFi.setScanMethod(WIFI_FAST_SCAN);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(m_ssid.c_str(), m_password.c_str());
        // TODO: call esp_wifi_set_storage(WIFI_STORAGE_RAM) after first connect to reduce flash wear
    }

    void Wifi::reconnect(const std::string& ssid, const std::string& pass)
    {
        Utils::Logger::printf("[WIFI] Reconnecting to '%s'\n", ssid.c_str());

        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(100);

        connect(ssid, pass);
    }

    void Wifi::disconnect()
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    bool Wifi::isConnected() const
    {
        return WiFi.status() == WL_CONNECTED;
    }

    std::string Wifi::localIP() const
    {
        return std::string(WiFi.localIP().toString().c_str());
    }

    int Wifi::rssi() const
    {
        return WiFi.RSSI();
    }

    void Wifi::toJson(JsonObject& doc)
    {
        doc["rssi"] = WiFi.RSSI();
        doc["local_ip"] = WiFi.localIP().toString();
        doc["connected"] = isConnected();
    }
}
