#include "EngineConfig.h"

using namespace Config;


void Engine::toJson(JsonObject& obj)
{
    // obj["to_factory_settings"] = to_factory_settings;
    obj["serial_report_task"] = serial_report_task;
    obj["wireless_report_task"] = wireless_report_task;
    obj["wifi_animation"] = wifi_animation;
    obj["ssid"] = ssid.c_str();
    obj["password"] = password.c_str();
}

std::string Engine::toString()
{
    JsonDocument obj;
    // obj["to_factory_settings"] = to_factory_settings;
    obj["serial_report_task"] = serial_report_task;
    obj["wireless_report_task"] = wireless_report_task;
    obj["wifi_animation"] = wifi_animation;
    obj["ssid"] = ssid.c_str();
    obj["password"] = password.c_str();

    std::string ret;
    serializeJson(obj, ret);
    return ret;
}