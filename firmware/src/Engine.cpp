#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Handlers/InputHandler.h"

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
        Utils::Logger::println("Error connecting");
    }
}

Engine& Engine::instance()
{
    static Engine engine;
    return engine;
}

void Engine::addButton(Input::Button&& button)
{
    ButtonManager::add(std::move(button));
}

Fixture* Engine::getFixture(uint16_t index)
{
    return FixtureHandler::get(index);
}

Fixture* Engine::operator[](uint16_t index)
{
    return getFixture(index);
}

void Engine::createTasks()
{
    xTaskCreate(Utils::Wifi::checkNetwork, "check wifi", 1000, NULL, 1 | portPRIVILEGE_BIT, NULL);
    xTaskCreate(Engine::update, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
}

void Engine::update(void*)
{
    while(true)
    {
        if(Utils::Wifi::isConnected())
        {
            InputHandler::update();

        }
        FixtureHandler::updateTask();

        Output::updateFastLED();
        vTaskDelay(20);
    }
}
