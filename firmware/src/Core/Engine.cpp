#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Utils/JsonUtils.h"

using namespace Utils::Json;

void Engine::Settings::toJson(JsonObject& obj)
{
    obj["print_task"] = print_task;
    obj["auto_report_task"] = report_task;
    obj["wifi_animation"] = wifi_animation;
    obj["ssid"] = ssid.c_str();
    obj["password"] = password.c_str();
}

std::string Engine::Settings::toString()
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

Engine::Engine ()
    : m_storage(Utils::Storage("engine"))
{

    Utils::Logger::enable();
    readSettings();
    // sleep(5);

    Utils::Wifi::initialize(settings.ssid.c_str(), settings.password.c_str(), [this](bool is_connected) 
    {
        if(inited)
        {
            this->m_taskExecutor.suspendTask(this->m_inputTaskID);
        }
        
        this->wifiStatus();
        
        if(inited)
        {
            this->m_taskExecutor.resumeTask(this->m_inputTaskID);
        }
        
        this->clearSrcBuffers();
    }, [this](std::string data)
    {
        this->parseConfig(data);
    });
}

void Engine::readSettings()
{
    if(m_storage.isKey("settings"))
    {
        std::string s = m_storage.getString("settings");
        JsonDocument doc;
        deserializeJson(doc, s);
        factoryReset = getElement<bool>(doc, "to_factory_settings", false);
        if(factoryReset)
        {
            settings.to_factory_settings = false;
            m_storage.putString("settings", settings.toString());
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
    if(!inited)
    {        
        m_inputTaskID = m_taskExecutor.spawnTask("DMX Input", [this]()
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

        // choose different port for this
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

    uint32_t animationHandle;
    if(this->settings.wifi_animation)
    {
        animationHandle = this->m_taskExecutor.spawnTask("Wifi animation", [this]()
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
        this->m_taskExecutor.stopTask(animationHandle);
    }
}


void Engine::parseConfig(const std::string& data)
{
    JsonDocument doc;
    deserializeJson(doc, data);
    
    std::string response;
    std::string status = "OK";

    auto sendResponse = [&response, &status]() 
    {
        JsonDocument json;
        json["status"] = status.c_str();
        json["response"] = response.c_str();

        std::string s;
        serializeJson(json, s);
        Utils::Wifi::instance().sendUdpPacket(12345, s);
    };
    

    JsonDocument jsonTemp;
    const char* req = doc["request"];

    if(strcmp(req, "describe") == 0)
    {
        response = Engine::instance().toString();
    }
    else if(strcmp(req, "fixtures") == 0)
    {
        JsonObject obj = jsonTemp["fixture_handler"].to<JsonObject>();
        m_fixtureHandler.toJsonFull(obj);
        serializeJson(jsonTemp, response);
    }
    else if(strcmp(req, "inputs") == 0)
    {
        JsonObject obj = jsonTemp["input_handler"].to<JsonObject>();
        m_inputHandler.toJson(obj);
        serializeJson(jsonTemp, response);
    }
    else if(strcmp(req, "reboot") == 0)
    {
        response = "Rebooting in 1 second";
        sendResponse();
        sleep(1);
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
            
            JsonObject obj = jsonTemp["wifi"].to<JsonObject>();
            Utils::Wifi::instance().toJson(obj);
            serializeJson(jsonTemp, response);
            status = "Wifi credentials set!";
        }
    }
    else if(strcmp(req, "setfixture") == 0)
    {
        if(doc["id"].is<uint8_t>())
        {
            uint8_t id = doc["id"];
            if(m_fixtureHandler.fixtureExists(id))
            {
                bool set = false;
                if (doc["universe"].is<uint8_t>())
                {
                    uint8_t universe = doc["universe"];
                    m_fixtureHandler.setFixtureUniverse(id, universe);
                    set = true;
                }
                if (doc["address"].is<uint16_t>())
                {
                    uint16_t address = doc["address"];
                    m_fixtureHandler.setFixtureAddress(id, address);
                    set = true;
                }
                if (doc["presetIndex"].is<uint8_t>())
                {
                    uint8_t presetIndex = doc["presetIndex"];
                    m_fixtureHandler.setFixturePreset(id, presetIndex);
                    set = true;
                }
                if (doc["name"].is<const char*>())
                {
                    const char* name = doc["name"];
                    m_fixtureHandler.setFixtureName(id, name);
                    set = true;
                }

                if(set)
                {
                    std::string key = "fixture" + std::to_string(id);
                    std::string store;

                    // TODO: make with Optional
                    serializeJson(m_fixtureHandler.get(id)->toJsonDoc(), store);
                    this->m_storage.putString(key, store);
                    status = "Fixture " + std::to_string(id) + " set";
                }
            }
            else
            {
                Utils::Logger::printf("[REQUEST] Could not find fixture with id %d\n", id);
                response = "Could not find selected fixture";
                status = "ERROR";
            }
        }
        else 
        {
            Utils::Logger::printf("[REQUEST] Set fixture json does not containd 'id' field!\n");
            response = "Invalid json";
            status = "ERROR";
        }
    }

    sendResponse();
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
    doc["version"] = ENGINE_VERSION;

    JsonObject settingsDoc = doc["settings"].to<JsonObject>();
    settings.toJson(settingsDoc);

    JsonObject wifiDoc = doc["wifi"].to<JsonObject>();
    Utils::Wifi::instance().toJson(wifiDoc);

    JsonObject fixtureDoc = doc["fixture_handler"].to<JsonObject>();
    m_fixtureHandler.toJson(fixtureDoc);

    // JsonObject inputDoc = doc["input_handler"].to<JsonObject>();
    // m_inputHandler.toJson(fixtureDoc);
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

    while(true)
    {
        std::string temp = toString().c_str();
        Utils::Wifi::instance().sendUdpPacket(12344, temp);

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
