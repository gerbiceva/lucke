#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Utils/JsonUtils.h"

using namespace Utils::Json;

void Engine::Settings::toJson(JsonObject& obj)
{
    obj["to_factory_settings"] = to_factory_settings;
    obj["print_task"] = print_task;
    obj["auto_report_task"] = report_task;
    obj["wifi_animation"] = wifi_animation;
    obj["ssid"] = ssid.c_str();
    obj["password"] = password.c_str();
}

std::string Engine::Settings::toString()
{
    JsonDocument obj;
    obj["to_factory_settings"] = to_factory_settings;
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
    : m_storage(Utils::Storage("engine")), m_inputHandler(Handler::InputHandler::instance())
{

    Utils::Logger::enable();
    Utils::Logger::setLevel(Utils::Logger::DEBUG);
    // sleep(5);
    readSettings();

    Utils::Wifi::initialize(settings.ssid.c_str(), settings.password.c_str(), [this](bool is_connected) 
    {
        this->m_taskExecutor.suspendTask(this->m_inputTaskID);        
        this->wifiStatus();        
        this->m_taskExecutor.resumeTask(this->m_inputTaskID);
        
        this->clearSrcBuffers();
    }, [this](std::string data)
    {
        this->parseConfig(data);
    });
}

void Engine::readSettings()
{
    // Utils::Logger::dprintln("[ENGINE] Reading settings");
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
        // Utils::Logger::dprintln("[ENGINE] Creating tasks");
        Utils::Wifi::startTasks();
        m_inputTaskID = m_taskExecutor.spawnTask("DMX Input", [this]()
        {
            this->m_inputHandler.update();
        },
        3, 20000);

        m_taskExecutor.spawnTask("Output", [this]()
        {
            this->m_fixtureHandler.update();
        },
        3, 20000);

        if(settings.print_task)
        {
            m_taskExecutor.spawnTask("Print Report", [this]()
            {
                this->printReport();
            }, 
            1, 10000);
        }

        // choose different port for this
        if(settings.report_task)
        {
            m_taskExecutor.spawnTask("Send report", [this]()
            {
                this->sendReport();
            }, 
            1, 10000);
        }

        inited = true;
    }
}

void Engine::wifiStatus()
{
    static bool firstTime = true;
    Utils::Logger::println("[WIFI] Disconnected...");

    uint32_t animationHandle;
    if(this->settings.wifi_animation || firstTime)
    {
        animationHandle = this->m_taskExecutor.spawnTask("Wifi animation", [this]()
        {
            this->clearSrcBuffers();
            while(true)
            {
                if(this->wifiAnimation != nullptr)
                {
                    this->wifiAnimation();
                }
                vTaskDelay(20);
            }
        }, 1, 1000);

    }

    while(!Utils::Wifi::instance().isConnected())
    {
        vTaskDelay(10);
    }
    
    Utils::Logger::println("[WIFI] Connected...");
    if(this->settings.wifi_animation || firstTime)
    {
        this->m_taskExecutor.stopTask(animationHandle);
    }

    firstTime = false;
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
        // Utils::Logger::dprintln("Describe engine request");
    }
    else if(strcmp(req, "fixtures") == 0)
    {
        JsonObject obj = jsonTemp["fixture_handler"].to<JsonObject>();
        m_fixtureHandler.toJsonFull(obj);
        serializeJson(jsonTemp, response);
        // Utils::Logger::dprintln("Describe fix handler request");
    }
    else if(strcmp(req, "inputs") == 0)
    {
        JsonObject obj = jsonTemp["input_handler"].to<JsonObject>();
        m_inputHandler.toJson(obj);
        serializeJson(jsonTemp, response);
        // Utils::Logger::dprintln("Describe input handler request");
    }
    else if(strcmp(req, "reboot") == 0)
    {
        response = "Rebooting in 1 second";
        // Utils::Logger::dprintln("Reboot request");
        sendResponse();
        sleep(1);
        ESP.restart();
    }
    else if(strcmp(req, "factory_reset") == 0)
    {
        std::string s = m_storage.getString("settings");
        JsonDocument doc;
        deserializeJson(doc, s);
        settings.to_factory_settings = true;
        m_storage.putString("settings", settings.toString());
        ESP.restart();
        return;
    }
    else if(strcmp(req, "print_task") == 0)
    {
        if(doc["value"].is<bool>())
        {
            bool value = doc["value"];
            settings.print_task = value;
            m_storage.putString("settings", settings.toString());
            response = Utils::String::concat("Set print task enabled to ", value);
        }
        else 
        {
            Utils::Logger::println("[REQUEST] Invalid json, missing field value");
            response = "Missing field value in print_task";
            status = "ERROR";
        }
    }
    else if(strcmp(req, "auto_report_task") == 0)
    {
        if(doc["value"].is<bool>())
        {
            bool value = doc["value"];
            settings.report_task = value;
            m_storage.putString("settings", settings.toString());
            response = Utils::String::concat("Set auto report task enabled to ", value);
        }
        else 
        {
            Utils::Logger::println("[REQUEST] Invalid json, missing field value");
            response = "Missing field value in auto_report_task";
            status = "ERROR";
        }
    }
    else if(strcmp(req, "wifi_animation") == 0)
    {
        if(doc["value"].is<bool>())
        {
            bool value = doc["value"];
            settings.wifi_animation = value;
            m_storage.putString("settings", settings.toString());
            response = Utils::String::concat("Set wifi animation enabled to ", value);
        }
        else 
        {
            Utils::Logger::println("[REQUEST] Invalid json, missing field value");
            response = "Missing field value in wifi animation";
            status = "ERROR";
        }
    }

    else if(strcmp(req, "wifi") == 0)
    {
        bool dirty = false;
        if(doc["ssid"].is<const char*>())
        {
            const char* ssid = doc["ssid"];
            settings.ssid = ssid;
            response = Utils::String::concat("Set ssid to '", ssid, "'");
            dirty = true;
            // Utils::Logger::dprintln("Set ssid request");
        }
        if(doc["password"].is<const char*>())
        {
            const char* password = doc["password"];
            settings.password = password;

            if(response.size() != 0)
            {
                response = Utils::String::concat(response, " and set password to '", password, "'");
            }
            else
            {
                response = Utils::String::concat("Set wifi password to '", password, "'");
            }

            dirty = true;
            // Utils::Logger::dprintln("Set password request");
        }

        if(dirty)
        {
            m_storage.putString("settings", settings.toString());
            sendResponse();
            // Utils::Wifi::reinitialize(settings.ssid.c_str(), settings.password.c_str());
            
            JsonObject obj = jsonTemp["wifi"].to<JsonObject>();
            Utils::Wifi::instance().toJson(obj);
            serializeJson(jsonTemp, response);
            // temp
            ESP.restart();
        }
    }    
    else if(strcmp(req, "getfixture") == 0)
    {
        if(doc["id"].is<uint8_t>())
        {
            uint8_t id = doc["id"];
            Fixture* fix = m_fixtureHandler.get(id);
            if(fix)
            {
                JsonObject o = jsonTemp["fixture"].to<JsonObject>();
                fix->toJsonFull(o);
                // Utils::Logger::dprintln("Get fixture");
                serializeJson(jsonTemp, response);
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
            Utils::Logger::printf("[REQUEST] Get fixture json does not containd 'id' field!\n");
            response = "Invalid json";
            status = "ERROR";
        }
    }
    else if(strcmp(req, "setfixture") == 0)
    {
        if(doc["id"].is<uint8_t>())
        {    
            uint8_t id = doc["id"];
            Fixture* fix = m_fixtureHandler.get(id);
            if(fix)
            {
                //Utils::Logger::dprintf("Set fixture %d\n", id);
                bool set = false;
                if (doc["universe"].is<uint8_t>())
                {
                    uint8_t universe = doc["universe"];
                    fix->setUniverse(universe);
                    response = Utils::String::concat("Set universe of fixture '", fix->getName(), "'[", fix->id(), "] to ", universe);
                    set = true;
                }
                if (doc["address"].is<uint16_t>())
                {
                    uint16_t address = doc["address"];
                    fix->setAddress(address);
                    response = Utils::String::concat("Set address of fixture '", fix->getName(), "'[", fix->id(), "] to ", address);
                    set = true;
                }
                if (doc["presetIndex"].is<uint8_t>())
                {
                    uint8_t presetIndex = doc["presetIndex"];
                    fix->setPreset(presetIndex);
                    response = Utils::String::concat("Set preset of fixture '", fix->getName(), "'[", fix->id(), "] to ", presetIndex);
                    set = true;
                }
                if (doc["name"].is<const char*>())
                {
                    const char* name = doc["name"];
                    fix->setName(name);
                    response = Utils::String::concat("Set name of fixture '", fix->getName(), "'[", fix->id(), "] to ", name);
                    set = true;
                }
                if (doc["highlight"].is<bool>())
                {
                    bool highlight = doc["highlight"];
                    m_fixtureHandler.highlightFixture(id);
                    response = Utils::String::concat("Toggled highlight for fixture '", fix->getName(), "'[", fix->id(), "] to ", highlight);
                }

                if(set)
                {
                    fix->updatePresets();

                    std::string key = "fixture" + std::to_string(id);
                    std::string store;

                    // TODO: make with Optional
                    serializeJson(fix->toJsonDoc(), store);
                    this->m_storage.putString(key, store);
                }
            }
            else
            {
                Utils::Logger::printf("[REQUEST] Could not find fixture with id %d\n", id);
                response = Utils::String::concat("Fixture with id '", id, "' does not exist");
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

    // Utils::Logger::dprintf("[REQUEST] %s\n", response);
    //Utils::Logger::dprintln(response.c_str());
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

    //Utils::Logger::dprintf("Added new button '%s' on pin %d\n", button.getName().c_str(), button.getPin());
    m_buttonManager.add(std::move(button));
}

std::shared_ptr<Traits::InputInterface> Engine::getDMXInput(uint8_t universe, uint8_t old_universe)
{
    return m_inputHandler.interface(universe, old_universe);
}

// Traits::InputInterface* Engine::getDMXInput(uint8_t universe)
// {
//     return m_inputHandler.interface(universe);
// }


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

    JsonObject inputDoc = doc["input_handler"].to<JsonObject>();
    m_inputHandler.toJson(inputDoc);
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
    std::string temp;
    while(true)
    {
        // JsonDocument doc;
        // JsonObject wifiDoc = doc["wifi"].to<JsonObject>();
        // Utils::Wifi::instance().toJson(wifiDoc);
        // serializeJson(doc, temp);
        std::string temp = toString().c_str();
        Utils::Wifi::instance().sendUdpPacket(12344, temp);

        vTaskDelay(10000);
    }
}

void Engine::printReport()
{
    while(true)
    {
        Utils::Logger::println(toString().c_str());
        vTaskDelay(10000);
    }
}
