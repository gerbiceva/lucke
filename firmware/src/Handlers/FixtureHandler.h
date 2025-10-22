#pragma once
// #include "../Lamp.h"
// #include <unordered_map>
#include <cstdint>
#include <vector>
#include "Fixture/Fixture.h"

class Fixture;

namespace Handler
{
    class Fixtures
    {
        static std::vector<Fixture*> m_fixtureConfigs;
        static WiFiUDP udp;
    public:
        static void addFixture(Fixture* ptr)
        {
            m_fixtureConfigs.push_back(ptr);
        }

        static void sendReport(void*)
        {
            while(true)
            {
                JsonDocument doc;
                doc["fixtures"] = JsonDocument();
                JsonArray jsonArray = doc["fixtures"].to<JsonArray>();
                for(Fixture* fix : m_fixtureConfigs)
                {
                    jsonArray.add(fix->toJson());
                }
    
                udp.beginPacket(WiFi.broadcastIP(), 12345);
                serializeJson(doc, udp);
                udp.endPacket();
            }
        }
    };
}