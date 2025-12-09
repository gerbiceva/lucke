#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Handlers/InputHandler.h"
#include "Handlers/ButtonManager.h"

#include <WiFi.h>


// Engine::Settings Engine::settings;
// std::function<void()> Engine::wifiAnimation = nullptr;

Fixture* Engine::wifiAnimFix = nullptr;

Engine::Engine ()
{
    // settings.setString("wifi_ssid", "ledique");
    // settings.setString("wifi_password", "dasenebipovezau");

    // settings.setShort("button_holdtime", 200);

    Utils::Logger::enable();
    Utils::Wifi::initialize("Ledique", "dasenebipovezau", [](bool is_connected) {
        
        if (is_connected) {
            Utils::Logger::println("[WIFI] Connected...");
            return;
        }

        Utils::Logger::println("[WIFI] Disconnected...");

    });

    //xTaskCreate(Engine::checkNetwork, "Check Wifi", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);
}

Engine& Engine::instance()
{
    static Engine engine;
    return engine;
}

void Engine::init()
{
    static bool inited = false;

    if(!inited)
    {
        // Create N tasks
        []() {

        };


        xTaskCreate([](void*){Engine::instance().updateInput(nullptr);}, "DMX Input", 4000, NULL, 3 | portPRIVILEGE_BIT, &m_inputHandle);
        xTaskCreate([](void*){Engine::instance().update(nullptr);}, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
        xTaskCreate([](void*){Engine::instance().sendReport(nullptr);}, "send report", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        // xTaskCreate([](void*){Engine::instance().printReport(nullptr);}, "print report", 3000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        inited = true;
    }
}

void Engine::clearSrcBuffers()
{
    Handler::InputHandler::clearSrcBuffers();
}


JsonDocument Engine::describe()
{
    JsonDocument doc;
    // doc["heap_size"] = ESP.getHeapSize();
	// doc["heap_free"] = ESP.getFreeHeap();
    doc["wifi"] = Utils::Wifi::instance().describe();
    doc["pin_handler"] = Handler::PinHandler::describe();
    doc["input_handler"] = Handler::InputHandler::describe();
    doc["fixture_handler"] = Handler::FixtureHandler::describe();
    return doc;
}

std::string Engine::toString()
{
    std::string ret;
    serializeJson(Engine::instance().describe(), ret);
    return ret;
}

void Engine::addButton(Input::Button&& button)
{
    Handler::ButtonManager::add(std::move(button));
}

void Engine::playIdleAnimation(void*)
{
    while(true)
    {
        if(Engine::instance().wifiAnimation)
        {
            Engine::instance().wifiAnimation();
            // Engine::animateWifi();
        }
        vTaskDelay(30);
    }
}

void Engine::checkNetwork(void *) {
    Utils::Logger::println("[TASK] Created 'WIFI check network' task");
    while (true) 
    {
        // if not connected
        if (!Utils::Wifi::instance().isConnected()) 
        {
            Utils::Logger::println("[WIFI] Disconnected, reconnecting...");
            // connected = false;
            // Handler::InputHandler::canUpdate(false);
            Engine::instance().canUpdate(false);

            TaskHandle_t animation = NULL;
            xTaskCreate(
                Engine::playIdleAnimation, 		// Task function
                "Animation",						// Name of the task (for debugging)
                2000,								// Stack size in words
                NULL,								// Parameter passed to the task
                1,									// Task priority
                &animation							// Handle to the task
            );

            // while not connected
            uint16_t counter = 0;
            while (!Utils::Wifi::instance().isConnected()) 
            {
                // if(counter++ % 10 == 0)
                // {
                //     Logger::print(".");
                // }
                vTaskDelay(10);
            }

            Utils::Logger::println("\n[WIFI] Connected");

            vTaskDelete(animation);
            // connected = true;
            Engine::instance().canUpdate(true);
            Engine::instance().clearSrcBuffers();
        }

        vTaskDelay(50);
    }
}

void Engine::updateInput(void*)
{
    Utils::Logger::println("[TASK] Created 'DMX Input' task!");

    while(true)
    {
        Handler::InputHandler::update();
        
        vTaskDelay(20);
    }
}

void Engine::update(void*)
{
    Utils::Logger::println("[TASK] Created 'DMX update' task");
    while(true)
    {
        Handler::FixtureHandler::update();
        Output::updateFastLED();
        vTaskDelay(20);
    }
}

void Engine::sendReport(void*)
{
    Utils::Logger::println("[TASK] Created 'Send report' task");

    WiFiUDP udp;
    while(true)
    {
        udp.beginPacket(WiFi.broadcastIP(), 12345);
        serializeJson(Engine::instance().describe(), udp);
        udp.endPacket();

        vTaskDelay(1000);
    }
}

void Engine::printReport(void*)
{
    Utils::Logger::println("[TASK] Created 'print report' task");

    while(true)
    {
        Utils::Logger::println(Engine::instance().toString().c_str());
        vTaskDelay(10000);
    }
}
