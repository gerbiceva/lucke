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
    
    void fixtureJson(JsonObject& doc);
    void settingsJson(JsonObject& doc);
    
    struct Settings
    {
        bool to_factory_settings = false;
        bool print_task = true;
        bool report_task = true;
        bool wifi_animation = true;
        std::string ssid = "Ledique";
        std::string password = "dasenebipovezau";
        
        std::string toString()
        {
            JsonDocument doc;
            doc["print_task"] = print_task;
            doc["auto_report_task"] = report_task;
            doc["wifi_animation"] = wifi_animation;
            doc["ssid"] = ssid.c_str();
            doc["password"] = password.c_str();
            // doc["to_factory_settings"] = to_factory_settings;
            std::string s;
            serializeJson(doc, s);
            return s;
        }
    } settings;
    
    const std::string version = "0.8";
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
        if(m_storage.isKey(key) && !factory)
        {
            fix->fromJson(m_storage.getString(key));
        }
        else
        {
            std::string temp;
            JsonObject doc;
            fix->toJson(doc);
            serializeJson(doc, temp);
            fix->fromJson(temp);
            m_storage.putString(key, temp);
        }

        return fix;
    }

    void addButton(Input::Button&& button);

    Traits::InputInterface* getDMXInput(uint8_t universe);
    void clearSrcBuffers();
    
    void toJson(JsonObject& doc) override;
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
    bool factory = false;

};