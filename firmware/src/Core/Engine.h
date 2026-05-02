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

#include "EngineSettings.h"

// #define DEFAULT_WIFI_SSID "lasportiva5"
// #define DEFAULT_WIFI_PASSWORD "GregorJeCar!56"

class Engine : public Traits::Deserializable
{
    Engine ();
    void readSettings();
    
    void wifiStatus();
    void parseConfig(const std::string& data, bool serial = false);
    
    void sendReport();
    void printReport();
    void readSerial();

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

        fix->updatePresets();

        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);
        m_storage.putString(key, serialized);

        // Utils::Logger::dprintf("[ENGINE] Added new fixture %s", fix->getName().c_str());
        return fix;
    }

    template<typename TFixture>
    Fixture* addFixture(std::string name, bool animateWifiConnecting = false)
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

        fix->updatePresets();

        JsonDocument doc = fix->toJsonDoc();
        std::string serialized;
        serializeJson(doc, serialized);
        m_storage.putString(key, serialized);

        // Utils::Logger::dprintf("[ENGINE] Added new fixture %s", fix->getName().c_str());
        return fix;
    }

    void addButton(Input::Button&& button);

    std::shared_ptr<Traits::InputInterface> getDMXInput(uint8_t universe, uint8_t old_universe);
    void clearSrcBuffers();
    
    void toJson(JsonObject& doc) override;
    std::string toString();

private:
    EngineSettings m_settings;

    Utils::Storage m_storage;
    Utils::TaskExecutor m_taskExecutor;

    Handler::FixtureHandler m_fixtureHandler;
    Handler::InputHandler& m_inputHandler;
    Handler::ButtonManager m_buttonManager;
    Handler::PinHandler m_pinManager;
    
    uint32_t m_inputTaskID;
    std::function<void()> wifiAnimation;
    
    bool inited = false;
};