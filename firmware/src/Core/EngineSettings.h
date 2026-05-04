#pragma once
#include "Traits/Deserializable.h"
#include <string>
#include <ArduinoJson.h>

#define ENGINE_VERSION "0.9"
#define DEFAULT_WIFI_SSID "Ledique"
#define DEFAULT_WIFI_PASSWORD "dasenebipovezau"


struct EngineSettings : public Traits::Deserializable
{
    bool to_factory_settings = false;
    bool print_task = true;
    bool report_task = true;
    bool wifi_animation = false;

    std::string ssid = DEFAULT_WIFI_SSID;
    std::string password = DEFAULT_WIFI_PASSWORD;
    
    void toJson(JsonObject& obj);
    std::string toString();

    // bool shouldPrint(bool v);
    // bool shouldSendReport(bool v);
    // bool shouldDisplayWifiAnimation(bool v);
};