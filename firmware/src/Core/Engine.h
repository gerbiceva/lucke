#pragma once
// #include "Fixtures.h"
#include "Handlers/FixtureHandler.h"
#include "Handlers/ButtonManager.h"
#include <string>
#include <unordered_map>
#include <functional>
#include "Traits/Serializable.h"

// #define ENGINE_VERSION "1.1"

class Engine : public Traits::Serializable
{
    // class Settings
    // {
    //     std::unordered_map<std::string, std::string> m_settings;

    // public:
    //     void setString(std::string key, std::string value)
    //     {
    //         m_settings[key] = value;
    //     }

    //     void setShort(std::string key, uint16_t number)
    //     {
    //         m_settings[key] = std::to_string(number);
    //     }

    //     uint16_t getShort(std::string key)
    //     {
    //         return std::atoi(m_settings[key].c_str());
    //     }
    // };
    
    TaskHandle_t m_inputHandle;

    Engine ();
    static void playIdleAnimation(void*);
    static void checkNetwork(void*);
    void updateInput(void*);
    void update(void*);
    void sendReport(void*);
    void printReport(void*);

    
public:
    std::function<void()> wifiAnimation;
    // static Settings settings;
    static Fixture* wifiAnimFix;
    static Engine& instance();

    template<typename TFixture>
    Fixture* addFixture(bool animateWifiConnecting = false)
    {
        Fixture* fix = Handler::FixtureHandler::addFixture<TFixture>();
        if(animateWifiConnecting)
        {
            wifiAnimation = [fix]() 
            {
                fix->wifiAnimation();
            };
            // animateWifi = &(fix->wifiAnimation);
        }

        return fix;
        // return nullptr;
    }

    // template<typename TFixture>
    // Fixture* addFixture(std::string name, std::string type, bool animateWifiConnecting = false)
    // {
    //     Fixture* fix = Handler::FixtureHandler::addFixture<TFixture>(name, type);
    //     if(animateWifiConnecting)
    //     {
    //         wifiAnimFix = fix;
    //     }

    //     return fix;
    // }

    void init();
    void canUpdate(bool b)
    {
        if(b)
        {
            vTaskResume(m_inputHandle);
        }
        else
        {
            vTaskSuspend(m_inputHandle);
        }
    }
    void clearSrcBuffers();
    void addButton(Input::Button&& button);
    
    JsonDocument describe() override;
    std::string toString();
};