#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"

#include <WiFi.h>


// Engine::Settings Engine::settings;
Engine::Engine ()
    : m_storage(Utils::Storage("engine"))
{
    // settings.setString("wifi_ssid", "ledique");
    // settings.setString("wifi_password", "dasenebipovezau");

    // settings.setShort("button_holdtime", 200);

    Utils::Logger::enable();
    // sleep(3);


    Utils::Wifi::initialize("Ledique", "dasenebipovezau", [this](bool is_connected) {
        Utils::Logger::println("[WIFI] Disconnected...");
        this->suspendInputTask();

        TaskHandle_t animation;
        if(this->shouldPlayWifiAnimation)
        {
            animation = this->m_taskExecutor.spawnTask("Wifi animation", [this]()
            {
                while(true)
                {
                    if(this->wifiAnimation != nullptr)
                    {
                        this->wifiAnimation();
                    }
                    vTaskDelay(40);
                }
            }, 1, 1000);
        }

        while(!Utils::Wifi::instance().isConnected())
        {
            vTaskDelay(10);
        }
        
        Utils::Logger::println("[WIFI] Connected...");
        if(this->shouldPlayWifiAnimation)
        {
            vTaskDelete(animation);
        }
        this->resumeInputTask();
        this->clearSrcBuffers();
    });

    if(m_storage.isKey("input_handler"))
    {
        m_inputHandler.fromJson(m_storage.getString("input_handler"));
    }
}

Engine& Engine::instance()
{
    static Engine engine;
    return engine;
}

void Engine::init()
{
    static bool inited = false;

    if(!inited)
    {
        // if(m_storage.isKey("input_handler"))
        // {
            // Utils::Logger::println(m_storage.getString("input_handler").c_str());
        //     m_storage.putString("input_handler", R"(
        //         {"inputs":[{"id":1,"universe":8,"type":"SACN","seq_diff":1}]})");

            // Utils::Logger::println(m_storage.getString("input_handler").c_str());

            // std::string temp = m_storage.getString("input_handler");
            // m_inputHandler.fromJson(temp);

        // }
        
        m_inputHandle = m_taskExecutor.spawnTask("DMX Input", [this]()
        {
            this->m_inputHandler.update();
        },
        3, 4000);

        m_taskExecutor.spawnTask("Output", [this]()
        {
            this->m_fixtureHandler.update();
        }, 
        3, 5000);

        m_taskExecutor.spawnTask("Print Report", [this]()
        {
            this->printReport();
        }, 
        1, 3000);
        // xTaskCreate([](void*){Engine::instance().sendReport(nullptr);}, "send report", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);
        inited = true;

        // syncToStorage();
    }
}

void Engine::addButton(Input::Button&& button)
{
    static bool enabled = false;
    if(!enabled) 
    {
        m_taskExecutor.spawnTask("Button Input", [this]()
        {
            this->m_buttonManager.update();
        }, 2);
        enabled = true;
    }

    m_buttonManager.add(std::move(button));
}

void Engine::resumeInputTask()
{
    vTaskResume(m_inputHandle);
}

void Engine::suspendInputTask()
{
    vTaskSuspend(m_inputHandle);
}

void Engine::syncToStorage()
{
    std::string temp;
    serializeJson(m_inputHandler.describe(), temp);
    m_storage.putString("input_handler", temp);
}


Traits::InputInterface* Engine::getDMXInput(uint8_t universe)
{
    return m_inputHandler.interface(universe);
}


void Engine::clearSrcBuffers()
{
    m_inputHandler.clearSrcBuffers();
}


JsonDocument Engine::describe()
{
    JsonDocument doc;
    // doc["heap_size"] = ESP.getHeapSize();
	// doc["heap_free"] = ESP.getFreeHeap();
    doc["wifi"] = Utils::Wifi::instance().describe();
    doc["pin_handler"] = Handler::PinHandler::describe();
    doc["input_handler"] = m_inputHandler.describe();
    doc["fixture_handler"] = m_fixtureHandler.describe();
    return doc;
}

std::string Engine::toString()
{
    std::string ret;
    serializeJson(describe(), ret);
    return ret;
}

void Engine::playIdleAnimation()
{
    while(true)
    {
        if(wifiAnimation)
        {
            wifiAnimation();
        }
        vTaskDelay(30);
    }
}


void Engine::sendReport(void*)
{
    Utils::Logger::println("[TASK] Created 'Send report' task");

    WiFiUDP udp;
    while(true)
    {
        udp.beginPacket(WiFi.broadcastIP(), 12345);
        serializeJson(Engine::instance().describe(), udp);
        udp.endPacket();

        vTaskDelay(1000);
    }
}

void Engine::printReport()
{
    Utils::Logger::println("[TASK] Created 'print report' task");

    while(true)
    {
        Utils::Logger::println(toString().c_str());
        vTaskDelay(10000);
    }
}
