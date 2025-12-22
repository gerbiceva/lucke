#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Utils/JsonUtils.h"

using namespace Utils::Json;

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
        factory = getElement<bool>(doc, "to_factory_settings", false);
        if(factory)
        {
            settings.to_factory_settings = false;
            m_storage.putString("settings", settings.toString());
            ESP.restart();
            return;
        }

        updateElement<bool>(doc, "print_task", settings.print_task);
        updateElement<bool>(doc, "auto_report_task", settings.report_task);
        updateElement<bool>(doc, "wifi_animation", settings.wifi_animation);

        const char* ssid = getElement<const char*>(doc, "ssid", settings.ssid.c_str());
        settings.ssid = ssid;
        const char* password = getElement<const char*>(doc, "password", settings.password.c_str());
        settings.password = password;
    }

    m_storage.putString("settings", settings.toString());
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

        // if(settings.report_task)
        // {
        //     m_taskExecutor.spawnTask("Send report", [this]()
        //     {
        //         this->sendReport();
        //     }, 
        //     1, 4000);
        // }

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
        JsonObject doc;
        Engine::instance().toJson(doc);
        serializeJson(doc, temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
    }
    else if(strcmp(req, "fixtures") == 0)
    {
        JsonObject doc;
        m_fixtureHandler.toJsonFull(doc);
        serializeJson(doc, temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
    }
    else if(strcmp(req, "inputs") == 0)
    {
        JsonObject doc;
        m_inputHandler.toJson(doc);
        serializeJson(doc, temp);
        Utils::Wifi::instance().sendUdpPacket(12345, temp);
    }
    else if(strcmp(req, "reboot") == 0)
    {
        JsonObject doc;
        status = "rebooting";
        Utils::Wifi::instance().toJson(doc);
        serializeJson(doc, temp);
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
        
        JsonObject doc;
        Utils::Wifi::instance().toJson(doc);
        serializeJson(doc, temp);
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

                    JsonObject doc;
                    fix->toJson(doc);
                    serializeJson(doc, store);
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

void Engine::toJson(JsonObject& doc)
{
    // doc["heap_size"] = ESP.getHeapSize();
	// doc["heap_free"] = ESP.getFreeHeap();
    doc["version"] = version.c_str();

    JsonObject settingsDoc = doc["settings"].to<JsonObject>();
    settings.toJson(settingsDoc);

    JsonObject wifiDoc = doc["wifi"].to<JsonObject>();
    Utils::Wifi::instance().toJson(wifiDoc);

    JsonObject fixtureDoc = doc["fixture_handler"].to<JsonObject>();
    m_fixtureHandler.toJson(fixtureDoc);
}

std::string Engine::toString()
{
    JsonDocument doc;
    JsonObject engineObj = doc["engine"].to<JsonObject>();
    toJson(engineObj);

    std::string ret;
    serializeJson(doc, ret);
    return ret;
}

void Engine::sendReport()
{
    Utils::Logger::println("[TASK] Created 'Send report' task");

    std::string temp;
    while(true)
    {
        JsonObject doc;
        m_fixtureHandler.toJson(doc);
        serializeJson(doc, temp);
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
