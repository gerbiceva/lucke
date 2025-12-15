#include "FixtureHandler.h"
#include "Utils/Wifi.h"
#include "Core/Fixture.h"
#include "Output/HardwareLED.h"

namespace Handler
{
    // std::vector<Fixture*> FixtureHandler::fixtures;
    
    // void FixtureHandler::addFixture(Fixture* fixture)
    // {
    //     fixtures.push_back(fixture);
    // }
    
    Fixture* FixtureHandler::get(uint8_t index)
    {
        if(index > fixtures.size())
        {
            Utils::Logger::printf("Error: Fixture at index %d does not exist/n", index);
            return nullptr;
        }
    
        return fixtures[index];
    }
    
    std::vector<Fixture*>& FixtureHandler::allFixtures()
    {
        return fixtures;
    }
    
    void FixtureHandler::update()
    {
         Utils::Logger::println("[TASK] Created 'DMX update' task");
        while(true)
        {
            for(Fixture* fix : fixtures)
            {
                fix->update();
            }
            Output::updateFastLED();
            vTaskDelay(20);
        }
    }

    void FixtureHandler::fromJson(std::string json) 
    {
        JsonDocument doc;
        deserializeJson(doc, json);

        JsonArray arr = doc["fixtures"].as<JsonArray>();
    }
    
    JsonDocument FixtureHandler::toJson()
    {
        JsonDocument doc;
        doc["num_fixtures"] = fixtures.size();
        doc["fixtures"] = JsonDocument();
        JsonArray arr = doc["fixtures"].to<JsonArray>();
    
        for(Fixture* f : fixtures)
        {
            arr.add(f->toJson());
        }
    
        return doc;
    }

    JsonDocument FixtureHandler::toJsonFull()
    {
        JsonDocument doc;
        doc["num_fixtures"] = fixtures.size();
        doc["fixtures"] = JsonDocument();
        JsonArray arr = doc["fixtures"].to<JsonArray>();
    
        for(Fixture* f : fixtures)
        {
            arr.add(f->toJsonFull());
        }
    
        return doc;
    }
}

