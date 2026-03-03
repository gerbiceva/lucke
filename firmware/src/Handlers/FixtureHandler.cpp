#include "FixtureHandler.h"
#include "Utils/Wifi.h"
#include "Core/Fixture.h"
#include "Output/HardwareLED.h"

namespace Handler
{   
    Fixture* FixtureHandler::get(uint8_t index)
    {
        if(index > fixtures.size())
        {
            Utils::Logger::printf("Error: Fixture at index %d does not exist/n", index);
            return nullptr;
        }
    
        return fixtures[index];
    }

    Utils::Optional<JsonDocument> FixtureHandler::fixtureJson(uint8_t id)
    {
        Fixture* fix = get(id);
        if(fix)
        {
            return fix->toJsonDoc();
        }

        return Utils::Optional<JsonDocument>();
    }


    Utils::Optional<std::string> FixtureHandler::setFixtureUniverse(uint8_t id, uint8_t universe) 
    {
        Fixture* fix = get(id);
        if(fix)
        {
            fix->setUniverse(universe);
            return fix->getName();
        }
        
        return Utils::Optional<std::string>();
    }

    Utils::Optional<std::string> FixtureHandler::setFixtureAddress(uint8_t id, uint16_t address) 
    {
        Fixture* fix = get(id);
        if(fix)
        {
            fix->setAddress(address);
            return fix->getName();
        }
        
        return Utils::Optional<std::string>();
    }

    Utils::Optional<std::string> FixtureHandler::setFixturePreset(uint8_t id, uint8_t preset) 
    {
        Fixture* fix = get(id);
        if(fix)
        {
            fix->setPreset(preset);
            return fix->getName();
        }
        return Utils::Optional<std::string>();
    }

    Utils::Optional<std::string> FixtureHandler::setFixtureName(uint8_t id, std::string name) 
    {
        Fixture* fix = get(id);
        if(fix)
        {
            fix->setName(name);
            return fix->getName();
        }
        return Utils::Optional<std::string>();        
    }

    Utils::Optional<std::string> FixtureHandler::highlightFixture(uint8_t id)
    {
        Fixture* fix = get(id);
        if(fix)
        {
            if(!fix->m_isHighlighted)
            {
                // Utils::Logger::dprintf("[FIX_HANDLER] Highlighting fixture: %d\n", id);
                fix->m_isHighlighted = true;
                fix->highlight();
                fix->update();
            }
            else
            {
                // Utils::Logger::dprintf("[FIX_HANDLER] Un-Highlighting fixture: %d\n", id);
                fix->m_isHighlighted = false;
            }
            return fix->getName();
        }
        return Utils::Optional<std::string>();
    }


    
    std::vector<Fixture*>& FixtureHandler::allFixtures()
    {
        return fixtures;
    }
    
    void FixtureHandler::update()
    {
        while(true)
        {
            for(Fixture* fix : fixtures)
            {
                if(!fix->m_isHighlighted)
                {
                    fix->update();
                }
            }
            // Output::updateFastLED();
            vTaskDelay(20);
        }
    }

    void FixtureHandler::fromJson(std::string json) 
    {
        JsonDocument doc;
        deserializeJson(doc, json);

        JsonArray arr = doc["fixtures"].as<JsonArray>();
    }
    
    void FixtureHandler::toJson(JsonObject& doc)
    {
        doc["num_fixtures"] = fixtures.size();
        doc["fixtures"] = JsonDocument();
        JsonArray arr = doc["fixtures"].to<JsonArray>();
    
        for(Fixture* f : fixtures)
        {
            JsonObject entry = arr.add<JsonObject>();
            f->toJson(entry);
        }
    
    }

    void FixtureHandler::toJsonFull(JsonObject& doc)
    {
        doc["num_fixtures"] = fixtures.size();
        JsonArray arr = doc["fixtures"].to<JsonArray>();
    
        for(Fixture* f : fixtures)
        {
            JsonObject entry = arr.add<JsonObject>();
            f->toJsonFull(entry);
        }
    }
}

