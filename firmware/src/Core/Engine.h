#pragma once
// #include "Fixtures.h"
#include <string>
#include <unordered_map>
#include <functional>

#include "Handlers/FixtureHandler.h"
#include "Handlers/InputHandler.h"
#include "Handlers/ButtonManager.h"
#include "Handlers/PinHandler.h"
#include "Utils/TaskExecutor.h"
#include "Utils/Storage.h"
#include "Traits/Serializable.h"

// #define ENGINE_VERSION "1.1"

class Engine : public Traits::Deserializable
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

    Engine ();

    void resumeInputTask();
    void suspendInputTask();

    void syncToStorage();
    void playIdleAnimation();
    void wirelessConfigTask();
    void sendReport();
    void printReport();
    
public:
    // static Settings settings;
    static Engine& instance();
    void init();

    template<typename TFixture>
    Fixture* addFixture(bool animateWifiConnecting = false)
    {
        Fixture* fix = m_fixtureHandler.addFixture<TFixture>();
        if(animateWifiConnecting)
        {
            wifiAnimation = [fix]() 
            {
                fix->wifiAnimation();
            };
        }

        std::string key = "fixture" + std::to_string(fix->id());
        if(m_storage.isKey(key))
        {
            fix->fromJson(m_storage.getString(key));
        }
        else
        {
            std::string temp;
            serializeJson(fix->toJson(), temp);
            fix->fromJson(temp);
            m_storage.putString(key, temp);
        }

        return fix;
    }

    void addButton(Input::Button&& button);

    Traits::InputInterface* getDMXInput(uint8_t universe);
    void parseConfig(const std::string& data);
    void clearSrcBuffers();
    
    JsonDocument fixtureJson();
    JsonDocument toJson() override;
    std::string toString();

private:
    Utils::TaskExecutor m_taskExecutor;
    Handler::FixtureHandler m_fixtureHandler;
    Handler::InputHandler m_inputHandler;
    Handler::ButtonManager m_buttonManager;
    Handler::PinHandler m_pinManager;
    
    TaskHandle_t m_inputHandle;
    std::function<void()> wifiAnimation;
    
    Utils::Storage m_storage;

    const bool shouldPlayWifiAnimation = true;

};