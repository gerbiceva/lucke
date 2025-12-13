#include "ButtonManager.h"
#include <Arduino.h>
#include "PinHandler.h"


namespace Handler
{
    void ButtonManager::update() 
    {
        while(true) 
        {
            for(auto& b : buttons) 
            {
                b.update();
            }
    
            vTaskDelay(20);
        }
    }
    
    void ButtonManager::add(Input::Button&& b) 
    {
        buttons.push_back(std::move(b));
    }

    JsonDocument ButtonManager::toJson()
    {
        JsonDocument doc;
        doc["buttons"] = JsonDocument();
        JsonArray arr = doc["buttons"].to<JsonArray>();

        for(Input::Button& button : buttons)
        {
            arr.add(button.toJson());
        }

        return doc;
    }
}

