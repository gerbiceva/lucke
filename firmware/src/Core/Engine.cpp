#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"


Engine::Engine ()
    : m_storage(Utils::Storage("engine"))
{

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
    }, [this](std::string data)
    {
        this->parseConfig(data);
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

        // m_taskExecutor.spawnTask("Send report", [this]()
        // {
        //     this->sendReport();
        // }, 
        // 1, 2000);
        inited = true;

    }
}

void Engine::parseConfig(const std::string& data)
{}

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
    serializeJson(m_inputHandler.toJson(), temp);
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


JsonDocument Engine::fixtureJson()
{
    JsonDocument doc;
    doc["fixtures"] = m_fixtureHandler.toJsonFull();
    return doc;
}


JsonDocument Engine::toJson()
{
    JsonDocument doc;
    // doc["heap_size"] = ESP.getHeapSize();
	// doc["heap_free"] = ESP.getFreeHeap();
    doc["wifi"] = Utils::Wifi::instance().toJson();
    doc["pin_handler"] = Handler::PinHandler::toJson();
    doc["input_handler"] = m_inputHandler.toJson();
    doc["fixture_handler"] = m_fixtureHandler.toJson();
    return doc;
}

std::string Engine::toString()
{
    std::string ret;
    serializeJson(toJson(), ret);
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

void Engine::wirelessConfigTask() {
    // WiFiServer server(8888);
    // server.begin();
    
    // while(true) {
    //     // Serial.println(WiFi.localIP());
    //     WiFiClient client = server.available();
    //     if (client) {
    //         while (client.connected()) {
    //             if (client.available()) {
    //                 // String name = client.readStringUntil(',');
    //                 String universe = client.readStringUntil(',');
    //                 String offset = client.readStringUntil(',');
    //                 String preset = client.readStringUntil('\n');
    //                 Serial.println("Universe: " + universe);
    //                 Serial.println("Offset: " + offset);
    //                 Serial.println("Preset: " + preset);
    //                 Controller::get().init(universe.toInt(), offset.toInt(), preset.toInt());
    //             }
    //         }
    //         client.stop();
    //     }
    //     vTaskDelay(100);
    // }
}

void Engine::sendReport()
{
    Utils::Logger::println("[TASK] Created 'Send report' task");

    std::string temp;
    while(true)
    {
        serializeJson(m_fixtureHandler.toJsonFull(), temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);

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
