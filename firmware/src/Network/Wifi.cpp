#include "Wifi.h"
#include "Utils/Logger.h"

#include <WiFi.h>
#include <esp_wifi.h>

namespace
{
    constexpr Utils::Log s_log{"WIFI"};
}

namespace Net
{
    void Wifi::connect(const std::string& ssid, const std::string& pass)
    {
        m_ssid = ssid;
        m_password = pass;

        s_log.info("connecting to '%s'", m_ssid.c_str());

        WiFi.setScanMethod(WIFI_FAST_SCAN);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(m_ssid.c_str(), m_password.c_str());
        // TODO: call esp_wifi_set_storage(WIFI_STORAGE_RAM) after first connect to reduce flash wear
    }

    void Wifi::reconnect(const std::string& ssid, const std::string& pass)
    {
        s_log.info("reconnecting to '%s'", ssid.c_str());

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

    WifiInfo Wifi::info() const
    {
        return {isConnected(), localIP(), rssi()};
    }

    void Wifi::toJson(JsonObject& doc)
    {
        info().toJson(doc);
    }
}
