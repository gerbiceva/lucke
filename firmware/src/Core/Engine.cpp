#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"


Engine::Engine ()
    : m_storage(Utils::Storage("engine"))
{

    Utils::Logger::enable();
    readSettings();
    sleep(5);
    // if(settings.print_task)
    // {
    // }


    Utils::Wifi::initialize(settings.ssid.c_str(), settings.password.c_str(), [this](bool is_connected) 
    {
        if(!is_connected)
        {
            this->suspendInputTask();
            this->wifiStatus();
            this->resumeInputTask();            
            this->clearSrcBuffers();
        }
    }, [this](std::string data)
    {
        this->parseConfig(data);
    });

    // if(m_storage.isKey("input_handler"))
    // {
    //     m_inputHandler.fromJson(m_storage.getString("input_handler"));
    // }
}

void Engine::readSettings()
{
    if(m_storage.isKey("settings"))
    {
        std::string s = m_storage.getString("settings");
        JsonDocument doc;
        deserializeJson(doc, s);
        factory = doc["to_factory_settings"];
        if(factory)
        {
            settings.to_factory_settings = false;
            m_storage.putString("settings", settings.toString());
            return;
        }

        settings.print_task = doc["print_task"];
        settings.report_task = doc["auto_report_task"];
        settings.wifi_animation = doc["wifi_animation"];
        // if(doc["ssid"])
        // const char* ssid = doc["ssid"];
        // settings.ssid = ssid;
        // const char* password = doc["password"];
        // settings.password = password;
    }
    else
    {
        m_storage.putString("settings", settings.toString());
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

        if(settings.print_task)
        {
            m_taskExecutor.spawnTask("Print Report", [this]()
            {
                this->printReport();
            }, 
            1, 3000);
        }

        if(settings.report_task)
        {
            m_taskExecutor.spawnTask("Send report", [this]()
            {
                this->sendReport();
            }, 
            1, 4000);
        }

        inited = true;

    }

}

void Engine::wifiStatus()
{
    Utils::Logger::println("[WIFI] Disconnected...");

    TaskHandle_t animation;
    if(this->settings.wifi_animation)
    {
        animation = this->m_taskExecutor.spawnTask("Wifi animation", [this]()
        {
            Utils::Logger::println("[TASK] Created 'Wifi animation' task");

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
    if(this->settings.wifi_animation)
    {
        vTaskDelete(animation);
    }
}


void Engine::parseConfig(const std::string& data)
{
    std::string code = "OK";
    std::string status = "success";
    
    JsonDocument doc;
    deserializeJson(doc, data);

    const char* req = doc["request"];
    std::string temp;
    if(strcmp(req, "describe") == 0)
    {
        serializeJson(Engine::instance().toJson(), temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
    }
    else if(strcmp(req, "fixtures") == 0)
    {
        serializeJson(m_fixtureHandler.toJsonFull(), temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
    }
    else if(strcmp(req, "inputs") == 0)
    {
        serializeJson(m_inputHandler.toJson(), temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
    }
    else if(strcmp(req, "reboot") == 0)
    {
        status = "rebooting";
        serializeJson(Utils::Wifi::instance().toJson(), temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
        ESP.restart();
    }
    // else if(strcmp(req, "factory_reset") == 0)
    // {
    //     std::string s = m_storage.getString("settings");
    //     JsonDocument doc;
    //     deserializeJson(doc, s);
    //     settings.to_factory_settings = true;
    //     m_storage.putString("settings", settings.toString());
    //     ESP.restart();
    //     return;
    // }

    else if(strcmp(req, "wifi") == 0)
    {
        bool dirty = false;
        if(doc["ssid"].is<const char*>())
        {
            const char* ssid = doc["ssid"];
            settings.ssid = ssid;
            dirty = true;
        }
        if(doc["password"].is<const char*>())
        {
            const char* password = doc["password"];
            settings.password = password;
            dirty = true;
        }

        if(dirty)
        {
            m_storage.putString("settings", settings.toString());
            Utils::Wifi::reinitialize(settings.ssid.c_str(), settings.password.c_str());
        }
        
        serializeJson(Utils::Wifi::instance().toJson(), temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
    }
    else if(strcmp(req, "setfixture") == 0)
    {
        if(doc["id"].is<uint8_t>())
        {
            int id = doc["id"];
            Fixture* fix = m_fixtureHandler.get(id);
            if(fix)
            {
                bool set = false;
                if (doc["universe"].is<uint8_t>())
                {
                    int universe = doc["universe"];
                    fix->setUniverse(universe);
                    set = true;
                }
                if (doc["address"].is<uint16_t>())
                {
                    int address = doc["address"];
                    fix->setAddress(address);
                    set = true;
                }
                if (doc["presetIndex"].is<uint8_t>())
                {
                    int presetIndex = doc["presetIndex"];
                    fix->setPreset(presetIndex);
                    set = true;
                }
                if (doc["name"].is<const char*>())
                {
                    const char* name = doc["name"];
                    fix->setName(name);
                    set = true;
                }

                if(set)
                {
                    std::string key = "fixture" + std::to_string(id);
                    std::string store;
                    serializeJson(fix->toJson(), store);
                    this->m_storage.putString(key, store);
                }
            }
            else
            {
                Utils::Logger::printf("[REQUEST] Could not find fixture with id %d\n", id);
                status = "Could not find selected fixture";
                code = "ERROR";
            }
        }
        else 
        {
            Utils::Logger::printf("[REQUEST] Set fixture json does not containd 'id' field!\n");
            status = "Invalid json";
            code = "ERROR";
        }
    }

    JsonDocument response;
    response["code"] = code.c_str();
    response["status"] = status.c_str();

    std::string s;
    serializeJson(response, s);
    Utils::Wifi::instance().sendUdpPacket(23456, s);
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

JsonDocument Engine::settingsJson()
{
    JsonDocument doc;
    if(m_storage.isKey("settings"))
    {
        std::string temp = m_storage.getString("settings");
        deserializeJson(doc, temp);
    }
    return doc;
}


JsonDocument Engine::toJson()
{
    JsonDocument doc;
    // doc["heap_size"] = ESP.getHeapSize();
	// doc["heap_free"] = ESP.getFreeHeap();
    doc["version"] = version.c_str();
    doc["settings"] = Engine::instance().settingsJson();
    doc["wifi"] = Utils::Wifi::instance().toJson();
    doc["fixture_handler"] = m_fixtureHandler.toJson();
    return doc;
}

std::string Engine::toString()
{
    std::string ret;
    serializeJson(toJson(), ret);
    return ret;
}

void Engine::sendReport()
{
    Utils::Logger::println("[TASK] Created 'Send report' task");

    std::string temp;
    while(true)
    {
        serializeJson(m_fixtureHandler.toJson(), temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);

        vTaskDelay(10000);
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
