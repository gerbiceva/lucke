#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Handlers/InputHandler.h"
#include "Handlers/ButtonManager.h"


Engine::Settings Engine::settings;
Fixture* Engine::wifiAnimFix = nullptr;

Engine::Engine ()
{
    settings.setString("wifi_ssid", "ledique");
    settings.setString("wifi_password", "dasenebipovezau");

    settings.setShort("button_holdtime", 200);

    Utils::Logger::enable();
    sleep(2);
    if(!Utils::Wifi::setup("ledique", "dasenebipovezau"))
    {
        // Utils::Logger::println("Error connecting");
        
    }

    xTaskCreate(Utils::Wifi::checkNetwork, "check wifi", 1000, NULL, 1 | portPRIVILEGE_BIT, NULL);
    xTaskCreate(Engine::update, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
    xTaskCreate(Engine::sendReport, "send report", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);
}

Engine& Engine::instance()
{
    static Engine engine;
    return engine;
}

void Engine::addButton(Input::Button&& button)
{
    Handler::ButtonManager::add(std::move(button));
}

Fixture* Engine::getFixture(uint16_t index)
{
    return Handler::FixtureHandler::get(index);
}

Fixture* Engine::operator[](uint16_t index)
{
    return getFixture(index);
}

void Engine::update(void*)
{
    while(true)
    {
        if(Utils::Wifi::isConnected())
        {
            Handler::InputHandler::update();

        }
        Handler::FixtureHandler::updateTask();

        Output::updateFastLED();
        vTaskDelay(20);
    }
}

void Engine::sendReport(void*)
{
    WiFiUDP udp;
    while(true)
    {
        if(Utils::Wifi::isConnected())
        {
            JsonDocument doc;
            doc["fix_handler"] = Handler::FixtureHandler::describe();
            doc["input_handler"] = Handler::InputHandler::describe();
            if(Handler::FixtureHandler::get(0) != nullptr)
            {
                udp.beginPacket(WiFi.broadcastIP(), 12345);
                serializeJson(doc, udp);
                udp.endPacket();
            }
        }

        vTaskDelay(1000);
    }
}
