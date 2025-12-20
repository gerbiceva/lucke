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

    void ButtonManager::toJson(JsonObject& doc)
    {
        JsonArray arr = doc.createNestedArray("buttons");

        for(Input::Button& button : buttons)
        {
            JsonObject entry = arr.createNestedObject();
            button.toJson(entry);
        }
    }
}

