#pragma once
#include <string>
#include <functional>
#include <atomic>

#include "Handlers/FixtureHandler.h"
#include "Handlers/InputHandler.h"
#include "Handlers/ButtonHandler.h"
#include "Handlers/PinHandler.h"

#include "Utils/TaskExecutor.h"
#include "Utils/Storage.h"
#include "Traits/Serializable.h"
#include "Config/EngineConfig.h"
#include "Fixtures/All.h"

class Engine : public Traits::Deserializable
{
    Engine ();
    void readSettings();
    
    void wifiStatus(bool connected);
    void parseConfig(const std::string& host, const std::string& data, bool serial = false);
    
    void ping();
    void animateConnecting();
    void sendReport();
    void printReport();
    void readSerial();

    void spawnSerialPrintTask(bool value, bool init = false);
    void spawnWirelessPrintTask(bool value, bool init = false);

    bool factoryReset = false;
    
public:
    std::atomic_bool should_reserialize;
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

    template<typename TFixture, typename... Args>
    Fixture* addFixture(bool animateWifiConnecting, Args&&... args)
    {
        Fixture* fix = m_fixtureHandler.addFixture<TFixture>(std::forward<Args>(args)...);
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
    JsonDocument basicDesc();
    std::string toString();

private:
    Config::Engine m_settings;
    Utils::Storage m_storage;
    Utils::TaskExecutor m_taskExecutor;

    Handler::FixtureHandler m_fixtureHandler;
    Handler::InputHandler& m_inputHandler;
    Handler::Button m_buttonManager;
    Handler::Pin m_pinManager;
    
    uint32_t m_animationTaskID;
    uint32_t m_inputTaskID;
    uint32_t m_sPrintTaskID = 0;
    uint32_t m_wPrintTaskID = 0;
    std::function<void()> wifiAnimation;
    
    bool inited = false;
};