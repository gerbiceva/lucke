#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Handlers/InputHandler.h"
#include "Handlers/ButtonManager.h"

#include <WiFi.h>


// Engine::Settings Engine::settings;
Fixture* Engine::wifiAnimFix = nullptr;

Engine::Engine ()
{
    // settings.setString("wifi_ssid", "ledique");
    // settings.setString("wifi_password", "dasenebipovezau");

    // settings.setShort("button_holdtime", 200);

    Utils::Logger::enable();
    sleep(5);

    if(!Utils::Wifi::setup("ledique", "dasenebipovezau"))
    {
        // Utils::Logger::println("Error connecting");
    }
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
        // xTaskCreate(Utils::Wifi::checkNetwork, "check wifi", 1000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        xTaskCreate(Engine::update, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
        // xTaskCreate(Engine::sendReport, "send report", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        xTaskCreate(Engine::printReport, "print report", 3000, NULL, 1 | portPRIVILEGE_BIT, NULL);
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
    doc["wifi"] = Utils::Wifi::describe();
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

void Engine::update(void*)
{
    Utils::Logger::println("[TASK] Created 'DMX update' task");
    while(true)
    {
        // if(Utils::Wifi::isConnected())
        // {
        //     Handler::InputHandler::update();
        // }

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
        if(Utils::Wifi::isConnected())
        {
            udp.beginPacket(WiFi.broadcastIP(), 12345);
            serializeJson(Engine::instance().describe(), udp);
            udp.endPacket();
        }

        vTaskDelay(100);
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
