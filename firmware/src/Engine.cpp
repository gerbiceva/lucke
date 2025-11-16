#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Handlers/InputHandler.h"

Fixture* Engine::wifiAnimFix = nullptr;

Engine::Engine ()
{
    Utils::Logger::enable();
    sleep(2);
    if(!Utils::Wifi::setup("ledique", "dasenebipovezau"))
    {
        Utils::Logger::println("Error connecting");
    }

    xTaskCreate(Utils::Wifi::checkNetwork, "check wifi", 1000, NULL, 1 | portPRIVILEGE_BIT, NULL);
    xTaskCreate(Engine::update, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
}

Engine& Engine::instance()
{
    static Engine engine;
    return engine;
}

Fixture* Engine::getFixture(uint16_t index)
{
    return FixtureHandler::get(index);
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
            InputHandler::update();

        }
        FixtureHandler::updateTask(nullptr);

        Output::updateFastLED();
        vTaskDelay(20);
    }
}
