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

#define ENGINE_VERSION "0.8"
#define DEFAULT_WIFI_SSID "Ledique"
#define DEFAULT_WIFI_PASSWORD "dasenebipovezau"
// #define DEFAULT_WIFI_SSID "lasportiva5"
// #define DEFAULT_WIFI_PASSWORD "GregorJeCar!56"

class Engine : public Traits::Deserializable
// class Engine 
{
    Engine ();
    void readSettings();
    
    void wifiStatus();
    void parseConfig(const std::string& data);
    
    void sendReport();
    void printReport();
    
    struct Settings : public Traits::Deserializable
    {
        bool to_factory_settings = false;
        bool print_task = true;
        bool report_task = true;
        bool wifi_animation = false;

        std::string ssid = DEFAULT_WIFI_SSID;
        std::string password = DEFAULT_WIFI_PASSWORD;
        
        void toJson(JsonObject& obj);
        std::string toString();
    } settings;

    bool factoryReset = false;
    
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
        if(m_storage.isKey(key) && !factoryReset)
        {
            fix->fromJson(m_storage.getString(key));
        }

        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);
        m_storage.putString(key, serialized);

        // Utils::Logger::dprintf("[ENGINE] Added new fixture %s", fix->getName().c_str());
        return fix;
    }

    void addButton(Input::Button&& button);

    Traits::InputInterface* getDMXInput(uint8_t universe);
    void clearSrcBuffers();
    
    void toJson(JsonObject& doc) override;
    std::string toString();

private:
    Utils::Storage m_storage;
    Utils::TaskExecutor m_taskExecutor;

    Handler::FixtureHandler m_fixtureHandler;
    Handler::InputHandler m_inputHandler;
    Handler::ButtonManager m_buttonManager;
    Handler::PinHandler m_pinManager;
    
    uint32_t m_inputTaskID;
    std::function<void()> wifiAnimation;
    
    bool inited = false;
};