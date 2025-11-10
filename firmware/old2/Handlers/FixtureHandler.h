#pragma once
// #include "../Lamp.h"
// #include <unordered_map>
#include <cstdint>
#include <vector>
#include <atomic>
#include "Fixture/Fixture.h"

class Fixture;

namespace Handler
{
    class Fixtures
    {
        static std::vector<Fixture*> m_fixtures;
        static WiFiUDP udp;
        static std::atomic<bool> connected;

        static void checkNetwork(void *) {
		// bool firstTime = true;
		// bool delayed = false;

            while (true) {
                // if not connected
                if (WiFi.status() != WL_CONNECTED) {
                    connected.store(false);

                    // filtering short disconnects
                    // if(!firstTime && !delayed) {
                    // 	vTaskDelay(WIFI_DISCONNECT_DELAY);
                    // 	delayed = true;
                    // 	continue;
                    // }

                    // TaskHandle_t animation = NULL;
                    // xTaskCreate(
                    // 	Controller::playIdleAnimation, 		// Task function
                    // 	"Animation",						// Name of the task (for debugging)
                    // 	2000,								// Stack size in words
                    // 	NULL,								// Parameter passed to the task
                    // 	1,									// Task priority
                    // 	&animation							// Handle to the task
                    // );

                    // while not connected
                    while (WiFi.status() != WL_CONNECTED) {
                        vTaskDelay(10);
                    }

                    // connection established
                    connected.store(true);
                    // vTaskDelete(animation);
                    // Controller::get().clear();
                    // firstTime = false;
                    // delayed = false;
                }

                vTaskDelay(50);
            }
        }
    public:

        static void addFixture(Fixture* ptr)
        {
            m_fixtures.push_back(ptr);
        }

        static void update(void*)
        {
            while(true)
            {
                if(!connected)
                {
                    continue;
                }
                Sacn::update();
                for(Fixture* fix : m_fixtures)
                {
                    fix->update();
                }
    
                Output::HardwareLED::updateFastLED();
                vTaskDelay(5);
            }
        }

        static void sendReport(void*)
        {
            while(true)
            {
                JsonDocument doc;
                doc["fixtures"] = JsonDocument();
                JsonArray jsonArray = doc["fixtures"].to<JsonArray>();
                for(Fixture* fix : m_fixtures)
                {
                    jsonArray.add(fix->toJson());
                }
    
                udp.beginPacket(WiFi.broadcastIP(), 12345);
                serializeJson(doc, udp);
                udp.endPacket();
                vTaskDelay(1000);
            }
        }
    };
}