#pragma once
// #include "Fixtures.h"
#include <string>
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
    Engine ();
    void readSettings();

    void resumeInputTask();
    void suspendInputTask();

    void wifiStatus();
    void parseConfig(const std::string& data);

    void sendReport();
    void printReport();

    JsonDocument fixtureJson();
    JsonDocument settingsJson();

    struct Settings
    {
        bool print_task = true;
        bool wifi_animation = true;
        std::string ssid = "Ledique";
        std::string password = "dasenebipovezau";

        std::string toString()
        {
            JsonDocument doc;
            doc["print_task"] = print_task;
            doc["wifi_animation"] = wifi_animation;
            doc["ssid"] = ssid.c_str();
            doc["password"] = password.c_str();
            std::string s;
            serializeJson(doc, s);
            return s;
        }
    } settings;
    
public:
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
    void clearSrcBuffers();
    
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