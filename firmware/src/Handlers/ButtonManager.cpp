#include "ButtonManager.h"
#include <Arduino.h>
#include "PinHandler.h"
// namespace Input
// {

std::vector<Input::Button> ButtonManager::buttons;

void ButtonManager::update(void*) 
{
    while(true) {
        for(auto& b : buttons) {
            b.update();
        }

        vTaskDelay(20);
    }
}

void ButtonManager::add(Input::Button&& b) 
{
    static bool enabled = false;
    if(!enabled) {
        xTaskCreate(ButtonManager::update, "Input", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
        enabled = true;
    }

    buttons.push_back(std::move(b));
}
