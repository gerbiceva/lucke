#pragma once
#include "Interfaces/Serializable.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <ArduinoJson.h>

inline constexpr std::string_view ENGINE_VERSION = "0.99";
inline constexpr std::string_view DEFAULT_WIFI_SSID = "Ledique";
inline constexpr std::string_view DEFAULT_WIFI_PASSWORD = "dasenebipovezau";

inline constexpr uint16_t COMMAND_PORT = 8888;
inline constexpr uint16_t HEARTBEAT_PORT = 12343;
inline constexpr uint16_t REPORT_PORT = 12344;
inline constexpr uint16_t OTA_PORT = 3232;
// Must match --auth in the *_ota env in platformio.ini.
inline constexpr std::string_view OTA_PASSWORD = "lucke-ota";

namespace Config
{

    struct Engine : public Interface::Serializable
    {
        static constexpr std::string_view version = ENGINE_VERSION;

        bool serial_report_task = true;
        bool wireless_report_task = true;
        bool wifi_animation = false;

        std::string ssid{DEFAULT_WIFI_SSID};
        std::string password{DEFAULT_WIFI_PASSWORD};

        // uint32_t serial_print_interval = 10000;
        // uint32_t wireless_print_interval = 10000;

        void toJson(JsonObject &obj) override;
        JsonDocument toJsonDoc() override;
        // Overwrites the fields present in obj; missing fields keep their value.
        void fromJson(JsonVariantConst obj);
        std::string toString();
    };

}