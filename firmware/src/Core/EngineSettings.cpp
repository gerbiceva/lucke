#include "EngineSettings.h"

void EngineSettings::toJson(JsonObject& obj)
{
    obj["to_factory_settings"] = to_factory_settings;
    obj["print_task"] = print_task;
    obj["auto_report_task"] = report_task;
    obj["wifi_animation"] = wifi_animation;
    obj["ssid"] = ssid.c_str();
    obj["password"] = password.c_str();
}

std::string EngineSettings::toString()
{
    JsonDocument obj;
    obj["to_factory_settings"] = to_factory_settings;
    obj["print_task"] = print_task;
    obj["auto_report_task"] = report_task;
    obj["wifi_animation"] = wifi_animation;
    obj["ssid"] = ssid.c_str();
    obj["password"] = password.c_str();

    std::string ret;
    serializeJson(obj, ret);
    return ret;
}