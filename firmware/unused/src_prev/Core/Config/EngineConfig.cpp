#include "EngineConfig.h"

using namespace Config;

void Engine::toJson(JsonObject& obj)
{
    obj["version"] = version;
    obj["serial_report_task"] = serial_report_task;
    obj["wireless_report_task"] = wireless_report_task;
    obj["wifi_animation"] = wifi_animation;
    obj["ssid"] = ssid.c_str();
    obj["password"] = password.c_str();
}

JsonDocument Engine::toJsonDoc()
{
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    toJson(obj);  // reuse the logic
    return doc;
}

std::string Engine::toString()
{
    std::string ret;
    serializeJson(toJsonDoc(), ret);
    return ret;
}