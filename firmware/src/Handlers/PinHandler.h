#pragma once
#include <cstdint>
#include <set>
#include <ArduinoJson.h>

namespace Handler
{
    class PinHandler
    {
        static std::set<uint8_t> m_usedPins;
    public:
        static bool available(uint8_t pin);
        static void toJson(JsonObject& doc);
    };
}
