#include <Arduino.h>
#include "ButtonHandler.h"
#include "PinHandler.h"


namespace Handler
{
    void Button::update() 
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
    
    void Button::add(Input::Button&& b) 
    {
        buttons.push_back(std::move(b));
    }

    void Button::toJson(JsonObject& doc)
    {
        JsonArray arr = doc["buttons"].as<JsonArray>();

        for(Input::Button& button : buttons)
        {
            JsonObject entry = arr.add<JsonObject>();
            button.toJson(entry);
        }
    }
}

