#pragma once
#include "Traits/Deserializable.h"
#include <string>
#include <ArduinoJson.h>

#define ENGINE_VERSION "0.9"

#define DEFAULT_WIFI_SSID "Ledique"
#define DEFAULT_WIFI_PASSWORD "dasenebipovezau"
// #define DEFAULT_WIFI_SSID "lasportiva5"
// #define DEFAULT_WIFI_PASSWORD "GregorJeCar!56"

#define HEARTBEAT_PORT 12343
#define REPORT_PORT 12344
#define RESPONSE_PORT 12345

namespace Config
{

struct Engine : public Traits::Deserializable
{
    const std::string version = ENGINE_VERSION;

    bool to_factory_settings = false;
    bool serial_report_task = true;
    bool wireless_report_task = true;
    bool wifi_animation = false;

    std::string ssid = DEFAULT_WIFI_SSID;
    std::string password = DEFAULT_WIFI_PASSWORD;

    // uint32_t serial_print_interval = 10000;
    // uint32_t wireless_print_interval = 10000;
    
    void toJson(JsonObject& obj) override;
    JsonDocument toJsonDoc() override;
    std::string toString();
};

}