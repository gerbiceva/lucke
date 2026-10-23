#pragma once
#include "Interfaces/Serializable.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <ArduinoJson.h>

inline constexpr std::string_view ENGINE_VERSION = "v0.98";
inline constexpr std::string_view DEFAULT_WIFI_SSID = "Ledique";
inline constexpr std::string_view DEFAULT_WIFI_PASSWORD = "dasenebipovezau";

// #define DEFAULT_WIFI_SSID "lasportiva5"
// #define DEFAULT_WIFI_PASSWORD "GregorJeCar!56"

inline constexpr uint16_t COMMAND_PORT = 8888;
inline constexpr uint16_t HEARTBEAT_PORT = 12343;
inline constexpr uint16_t REPORT_PORT = 12344;
inline constexpr uint16_t RESPONSE_PORT = 12345;

namespace Config
{

    struct Engine : public Interface::Serializable
    {
        static constexpr std::string_view version = ENGINE_VERSION;

        bool to_factory_settings = false;
        bool serial_report_task = true;
        bool wireless_report_task = true;
        bool wifi_animation = false;

        std::string ssid{DEFAULT_WIFI_SSID};
        std::string password{DEFAULT_WIFI_PASSWORD};

        // uint32_t serial_print_interval = 10000;
        // uint32_t wireless_print_interval = 10000;

        void toJson(JsonObject &obj) override;
        JsonDocument toJsonDoc() override;
        std::string toString();
    };

}