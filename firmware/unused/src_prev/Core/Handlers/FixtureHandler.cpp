#include "FixtureHandler.h"
#include "Utils/Wifi.h"
#include "Core/Fixture.h"
#include "Output/HardwareLED.h"

namespace Handler
{   
    Fixture* FixtureHandler::configureAndAddFixture(Fixture* fix, bool factoryReset)
    {
        std::string key = "fixture" + std::to_string(fix->id());
            // sleep(5);
        if(m_storage.isKey(key) && !factoryReset)
        {
            fix->fromJson(m_storage.getString(key));
        }
        else 
        {
            fix->fromConfig();
        }

        fixtures.push_back(fix);

        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);
        m_storage.putString(key, serialized);
        return fix;
    }
    void FixtureHandler::updateFixtureStorage(uint8_t fid)
    {
        Fixture* fix = get(fid);
        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);

        std::string key = "fixture" + std::to_string(fix->id());
        m_storage.putString(key, serialized);
    }

    bool FixtureHandler::exists(uint8_t index)
    {
        return index < fixtures.size();
    }


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
        if(fix && universe > 0)
        {
            fix->setUniverse(universe);
            fix->updatePresets();
            updateFixtureStorage(id);
            return fix->getName();
        }
        
        return Utils::Optional<std::string>();
    }

    Utils::Optional<std::string> FixtureHandler::setFixtureAddress(uint8_t id, uint16_t address) 
    {
        Fixture* fix = get(id);
        if(fix && address < 512)
        {
            fix->setAddress(address);
            fix->updatePresets();
            updateFixtureStorage(id);
            return fix->getName();
        }
        
        return Utils::Optional<std::string>();
    }

    Utils::Optional<std::string> FixtureHandler::setFixturePreset(uint8_t id, uint8_t preset) 
    {
        Fixture* fix = get(id);
        auto n = fix->getNumPresets();
        if(fix && preset < n)
        {
            fix->setPreset(preset);
            fix->updatePresets();
            updateFixtureStorage(id);
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
            updateFixtureStorage(id);
            return fix->getName();
        }
        return Utils::Optional<std::string>();        
    }

    Utils::Optional<std::string> FixtureHandler::highlightFixture(uint8_t id)
    {
        Fixture* fix = get(id);
        if(fix)
        {
            if(!fix->getHighlighted())
            {
                // Utils::Logger::dprintf("[FIX_HANDLER] Highlighting fixture: %d\n", id);
                fix->getHighlighted() = true;
                fix->highlight();
                fix->update();
            }
            else
            {
                // Utils::Logger::dprintf("[FIX_HANDLER] Un-Highlighting fixture: %d\n", id);
                fix->getHighlighted() = false;
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
                if(!fix->getHighlighted())
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

    JsonDocument FixtureHandler::presetsJson()
    {
        JsonDocument doc;
        JsonArray arr = doc["presets"].to<JsonArray>();

        const auto& fixtures = allFixtures();
        for(Fixture* f : fixtures)
        {
            JsonObject entry = arr.add<JsonObject>();
            f->getPresets(entry);
        }

        return doc;
    }
}

