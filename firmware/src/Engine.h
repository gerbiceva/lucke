#pragma once
#include "Fixtures.h"
#include "Handlers/FixtureHandler.h"
#include "Handlers/ButtonManager.h"
#include <string>
#include <unordered_map>

class Engine
{
    class Settings
    {
        std::unordered_map<std::string, std::string> m_settings;

    public:
        void setString(std::string key, std::string value)
        {
            m_settings[key] = value;
        }

        void setShort(std::string key, uint16_t number)
        {
            m_settings[key] = std::to_string(number);
        }

        uint16_t getShort(std::string key)
        {
            return std::atoi(m_settings[key].c_str());
        }
    };
    
    Engine ();
public:
    static Settings settings;
    static Fixture* wifiAnimFix;
    static Engine& instance();

    template<typename TFixture>
    Fixture* addFixture(bool animateWifiConnecting = false)
    {
        Fixture* fix = FixtureHandler::addFixture<TFixture>();
        if(animateWifiConnecting)
        {
            wifiAnimFix = fix;
        }

        return fix;
    }

    void addButton(Input::Button&& button);

    Fixture* getFixture(uint16_t index);
    Fixture* operator[](uint16_t index);

    void createTasks();
    static void update(void*);
};