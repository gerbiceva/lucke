#include "ButtonManager.h"
#include <Arduino.h>
#include "PinHandler.h"


namespace Handler
{
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
            xTaskCreate(ButtonManager::update, "Button Input", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
            enabled = true;
        }
    
        buttons.push_back(std::move(b));
    }

    JsonDocument ButtonManager::describe()
    {
        JsonDocument doc;
        doc["buttons"] = JsonDocument();
        JsonArray arr = doc["buttons"].to<JsonArray>();

        for(Input::Button& button : buttons)
        {
            arr.add(button.describe());
        }

        return doc;
    }
}

