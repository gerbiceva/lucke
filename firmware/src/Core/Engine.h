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
// class Engine 
{
    Engine ();
    void readSettings();
    
    void resumeInputTask();
    void suspendInputTask();
    
    void wifiStatus();
    void parseConfig(const std::string& data);
    
    void sendReport();
    void printReport();
    

    // template<typename T>
    // T getElement(JsonDocument& doc, std::string name, T alternative)
    // {
    //     if(doc[name].is<T>())
    //     {
    //         return doc[name];
    //     }

    //     return alternative;
    // }

    // template<typename T>
    // void updateElement(JsonDocument& doc, std::string name, T& element)
    // {
    //     if(doc[name].is<T>())
    //     {
    //         element = doc[name];
    //     }
    // }
    
    struct Settings : public Traits::Deserializable
    {
        bool to_factory_settings = false;
        bool print_task = true;
        bool report_task = true;
        bool wifi_animation = true;
        std::string ssid = "Ledique";
        std::string password = "dasenebipovezau";
        
        void toJson(JsonObject& obj) override
        {
            obj["print_task"] = print_task;
            obj["auto_report_task"] = report_task;
            obj["wifi_animation"] = wifi_animation;
            obj["ssid"] = ssid.c_str();
            obj["password"] = password.c_str();
        }

        std::string toString()
        {
            JsonDocument obj;
            obj["print_task"] = print_task;
            obj["auto_report_task"] = report_task;
            obj["wifi_animation"] = wifi_animation;
            obj["ssid"] = ssid.c_str();
            obj["password"] = password.c_str();

            std::string ret;
            serializeJson(obj, ret);
            return ret;
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

        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);
        m_storage.putString(key, serialized);



        // else
        // {
        //     std::string temp;
        //     JsonObject doc;
        //     fix->toJson(doc);
        //     serializeJson(doc, temp);
        //     fix->fromJson(temp);
        //     m_storage.putString(key, temp);
        // }

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