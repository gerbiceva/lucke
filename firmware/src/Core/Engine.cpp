#include "Engine.h"
#include "Utils/Logger.h"
#include "Utils/Wifi.h"
#include "Utils/JsonUtils.h"

using namespace Utils::Json;

Engine::Engine ()
    : m_storage(Utils::Storage("engine")), m_inputHandler(Handler::InputHandler::instance())
{

    Utils::Logger::enable();
    Utils::Logger::setLevel(Utils::Logger::DEBUG);
    readSettings();

    Utils::Wifi::initialize(m_settings.ssid.c_str(), m_settings.password.c_str(), [this](bool is_connected) 
    {
        if(this->wifiAnimation == nullptr)
        {
            return;
        }

        this->wifiStatus(is_connected);        
    }, [this](std::string ip, std::string data)
    {
        this->parseConfig(ip, data);
    });
}

void Engine::readSettings()
{
    // Utils::Logger::dprintln("[ENGINE] Reading settings");
    if(m_storage.isKey("settings"))
    {
        std::string s = m_storage.getString("settings");
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, s);
        if(err)
        {
            Utils::Logger::println("[ERROR] Engine: read settings json");
            return;
        }
        factoryReset = getElement<bool>(doc, "to_factory_settings", false);
        if(factoryReset)
        {
            m_settings.to_factory_settings = false;
            m_storage.putString("settings", m_settings.toString());
            s = m_storage.getString("settings");
            deserializeJson(doc, s);
            // return;
        }

        updateElement<bool>(doc, "serial_report_task", m_settings.serial_report_task);
        updateElement<bool>(doc, "wireless_report_task", m_settings.wireless_report_task);
        updateElement<bool>(doc, "wifi_animation", m_settings.wifi_animation);

        const char* ssid = getElement<const char*>(doc, "ssid", m_settings.ssid.c_str());
        m_settings.ssid = ssid;
        const char* password = getElement<const char*>(doc, "password", m_settings.password.c_str());
        m_settings.password = password;
    }
    else 
    {
        m_storage.putString("settings", m_settings.toString());
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
        should_reserialize = true;

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

        spawnSerialPrintTask(false, true);
        spawnWirelessPrintTask(false, true);

        m_taskExecutor.spawnTask("Heartbeat", [this]()
        {
            this->ping();
        },
        1, 1000);

        m_taskExecutor.spawnTask("Read serial", [this]()
        {
            this->readSerial();
        }, 
        2, 10000);


        inited = true;
    }
}


void Engine::wifiStatus(bool is_connected)
{
    static bool firstTime = true;
    static bool prev = is_connected;

    if(!is_connected && this->m_taskExecutor.isTaskRunning(this->m_inputTaskID))
    {
        this->m_taskExecutor.suspendTask(this->m_inputTaskID);
    }

    if(firstTime)
    {
        this->m_animationTaskID = this->m_taskExecutor.spawnTask("Wifi animation", [this]()
        {
            this->animateConnecting();
        }, 1, 1000);
    }

    else if(prev != is_connected)
    {   
        if(is_connected)
        {
            this->m_taskExecutor.suspendTask(this->m_animationTaskID);
            this->m_taskExecutor.resumeTask(this->m_inputTaskID);
            this->clearSrcBuffers();
        }
        else
        {
            this->m_taskExecutor.suspendTask(this->m_inputTaskID);
            if(this->m_settings.wifi_animation)
            {
                this->m_taskExecutor.resumeTask(this->m_animationTaskID);
            }
        }
    }

    prev = is_connected;
    firstTime = false;
}

void Engine::parseConfig(const std::string& host, const std::string& data, bool serial)
{
    if(data.length() == 1)
    {
        return;
    }

    std::string response;
    std::string status = "OK";

    auto sendResponse = [&host, &response, &status, serial]() 
    {
        JsonDocument json;
        json["status"] = status.c_str();
        json["response"] = response.c_str();

        std::string s;
        serializeJson(json, s);

        if(serial)
        {
            Utils::Logger::print("\nResponse: ");
            Utils::Logger::println(s.c_str());
            return;
        }
        // Utils::Logger::printf("Sending packet to: %s\n", host.c_str());
        // Utils::Wifi::instance().sendUdpPacket(RESPONSE_PORT, s);
        // s += "TCP";
        Utils::Wifi::instance().sendTcpPacket(host, RESPONSE_PORT, s);
    };

    JsonDocument doc;
    DeserializationError jsonError = deserializeJson(doc, data);

    if(jsonError)
    {
        response = Utils::String::concat("Invalid json:", "\n",  data);
        status = "ERROR";
        sendResponse();
        return;
    }
    

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
    else if(strcmp(req, "presets") == 0)
    {
        auto obj = m_fixtureHandler.presetsJson();
        serializeJson(obj, response);
    }
    else if(strcmp(req, "inputs") == 0)
    {
        JsonObject obj = jsonTemp["input_handler"].to<JsonObject>();
        m_inputHandler.toJson(obj);
        serializeJson(jsonTemp, response);
    }
    else if(strcmp(req, "ip") == 0)
    {
        auto v = Utils::Wifi::instance().getIP();
        if(v.has_value())
        {
            response = v.value();
        }
        else 
        {
            response = "No IP?";
        }
    }
    else if(strcmp(req, "reboot") == 0)
    {
        response = "Rebooting in 1 second";
        sendResponse();
        sleep(1);
        ESP.restart();
    }
    else if(strcmp(req, "factory_reset") == 0)
    {
        std::string s = m_storage.getString("settings");
        JsonDocument doc;
        deserializeJson(doc, s);
        m_settings.to_factory_settings = true;
        m_storage.putString("settings", m_settings.toString());
        ESP.restart();
        return;
    }
    else if(strcmp(req, "serial_report_task") == 0)
    {
        if(doc["value"].is<bool>())
        {
            bool value = doc["value"];
            spawnSerialPrintTask(value);

            m_settings.serial_report_task = value;
            m_storage.putString("settings", m_settings.toString());
            response = Utils::String::concat("Set serial report enabled to ", value);
        }
        else 
        {
            Utils::Logger::println("[REQUEST] Invalid json, missing field value");
            response = "Serial report is set to: " + std::to_string(m_settings.serial_report_task);
        }
    }
    else if(strcmp(req, "wireless_report_task") == 0)
    {
        if(doc["value"].is<bool>())
        {
            bool value = doc["value"];
            spawnWirelessPrintTask(value);

            m_settings.wireless_report_task = value;
            m_storage.putString("settings", m_settings.toString());
            response = Utils::String::concat("Set wireless report enabled to ", value);
        }
        else 
        {
            Utils::Logger::println("[REQUEST] Invalid json, missing field value");
            response = "Wireless report is set to: " + std::to_string(m_settings.wireless_report_task);
        }
    }
    else if(strcmp(req, "wifi_animation") == 0)
    {
        if(doc["value"].is<bool>())
        {
            bool value = doc["value"];
            m_settings.wifi_animation = value;
            m_storage.putString("settings", m_settings.toString());
            response = Utils::String::concat("Set wifi animation enabled to ", value);
        }
        else 
        {
            Utils::Logger::println("[REQUEST] Invalid json, missing field value");
            response = "Wifi animation is set to: " + std::to_string(m_settings.wifi_animation);
        }
    }

    else if(strcmp(req, "wifi") == 0)
    {
        bool dirty = false;
        if(doc["ssid"].is<const char*>())
        {
            const char* ssid = doc["ssid"];
            m_settings.ssid = ssid;
            response = Utils::String::concat("Set ssid to '", ssid, "'");
            dirty = true;
        }
        if(doc["password"].is<const char*>())
        {
            const char* password = doc["password"];
            m_settings.password = password;

            if(response.size() != 0)
            {
                response = Utils::String::concat(response, " and set password to '", password, "'");
            }
            else
            {
                response = Utils::String::concat("Set wifi password to '", password, "'");
            }

            dirty = true;
        }

        if(dirty)
        {
            m_storage.putString("settings", m_settings.toString());
            
            JsonObject obj = jsonTemp["wifi"].to<JsonObject>();
            Utils::Wifi::instance().toJson(obj);
            serializeJson(jsonTemp, response);
            sendResponse();
            sleep(1);
            Utils::Wifi::instance().reinitialize(m_settings.ssid.c_str(), m_settings.password.c_str());
            m_inputHandler.reconnectInputs();
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
                auto v = doc["value"].is<const char*>();
                if(v)
                {
                    std::string request = doc["value"];
                    JsonDocument doc;
                    if(request == "universe")
                    {
                        doc["request"] = "universe";
                        doc["value"] = fix->getUniverse();
                        serializeJson(doc, response);
                    }
                    else if(request == "address")
                    {
                        doc["request"] = "address";
                        doc["value"] = fix->getAddress();
                        serializeJson(doc, response);
                    }
                    else if(request == "preset")
                    {
                        doc["request"] = "preset";
                        doc["value"] = fix->getSelectedPreset();
                        serializeJson(doc, response);
                    }
                    else if(request == "name")
                    {
                        doc["request"] = "name";
                        doc["value"] = fix->getName();
                        serializeJson(doc, response);
                    }
                    else if(request == "type")
                    {
                        doc["request"] = "type";
                        doc["value"] = fix->getType();
                        serializeJson(doc, response);
                    }
                    else if(request == "config")
                    {
                        doc["request"] = "config";
                        doc["value"] = fix->configJson();
                        serializeJson(doc, response);
                    }
                }
                // JsonObject o = jsonTemp["fixture"].to<JsonObject>();
                // fix->toJsonFull(o);
                // serializeJson(jsonTemp, response);
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
                bool set = false;
                if (doc["universe"].is<uint8_t>())
                {
                    uint8_t universe = doc["universe"];
                    if(universe < 1)
                    {
                        response = Utils::String::concat("Cannot set universe of fixture '", fix->getName(), "'[", std::to_string(fix->id()), "] to ", universe);    
                        status = "ERROR";
                    }
                    else
                    {
                        fix->setUniverse(universe);
                        set = true;
                    }
                }
                if (doc["address"].is<uint16_t>())
                {
                    uint16_t address = doc["address"];
                    if(address >= 512)
                    {
                        response = Utils::String::concat("Cannot set address of fixture '", fix->getName(), "'[", std::to_string(fix->id()), "] to ", address);    
                        status = "ERROR";
                    }
                    else
                    {
                        fix->setAddress(address);
                        set = true;
                    }
                }
                if (doc["presetIndex"].is<uint8_t>())
                {
                    uint8_t presetIndex = doc["presetIndex"];
                    if(presetIndex >= fix->getNumPresets())
                    {
                        response = Utils::String::concat("Cannot set preset of fixture '", fix->getName(), "'[", std::to_string(fix->id()), "] to ", presetIndex);    
                        status = "ERROR";
                    }
                    else
                    {
                        fix->setPreset(presetIndex);
                        set = true;
                    }
                }
                if (doc["name"].is<const char*>())
                {
                    const char* name = doc["name"];
                    fix->setName(name);
                    set = true;
                    should_reserialize = true;
                }
                if (doc["highlight"].is<bool>())
                {
                    bool highlight = doc["highlight"];
                    m_fixtureHandler.highlightFixture(id);
                    response = Utils::String::concat("Toggled highlight for fixture '", fix->getName(), "'[", std::to_string(fix->id()), "] to ", highlight);
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
        }, 2, 1000);
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

JsonDocument Engine::basicDesc()
{
    JsonDocument doc;
    doc["version"] = m_settings.version;
    JsonArray arr = doc["fixtures"].to<JsonArray>();

    const auto& fixtures = m_fixtureHandler.allFixtures();
    for(Fixture* f : fixtures)
    {
        JsonObject entry = arr.add<JsonObject>();
        entry["name"] = f->getName();
        entry["type"] = f->getType();
    }

    return doc;
}

void Engine::toJson(JsonObject& doc)
{
    // doc["heap_size"] = ESP.getHeapSize();
	// doc["heap_free"] = ESP.getFreeHeap();
    // m_settings.toJson(doc);
    JsonObject settingsDoc = doc["settings"].to<JsonObject>();
    m_settings.toJson(settingsDoc);

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

void Engine::ping()
{
    std::string temp;
    while(true)
    {
        if(should_reserialize)
        {
            serializeJson(basicDesc(), temp);
            should_reserialize = false;
        }
        Utils::Wifi::instance().sendUdpPacket(HEARTBEAT_PORT, temp);
        vTaskDelay(500);
    }
}

void Engine::animateConnecting()
{
    this->clearSrcBuffers();
    while(true)
    {
        this->wifiAnimation();
        vTaskDelay(20);
    }
}

void Engine::sendReport()
{
    std::string temp;
    while(true)
    {
        std::string temp = toString().c_str();
        Utils::Wifi::instance().sendUdpPacket(REPORT_PORT, temp);

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

void Engine::readSerial()
{
    while(true)
    {
        if(Serial.available())
        {
            auto v = Serial.readStringUntil('\n');
            parseConfig("Serial0", std::string(v.c_str()), true);
        }

        vTaskDelay(1000);
    }
}

void Engine::spawnSerialPrintTask(bool value , bool init)
{
    if((init && m_settings.serial_report_task) || (value && !m_settings.serial_report_task))
    {
        m_sPrintTaskID = m_taskExecutor.spawnTask("Print Report", [this]()
        {
            this->printReport();
        }, 
        1, 10000);
        return;
    }
    else if(!value && m_settings.serial_report_task)
    {
        m_taskExecutor.stopTask(m_sPrintTaskID);
    }
}

void Engine::spawnWirelessPrintTask(bool value, bool init)
{
    if((init && m_settings.wireless_report_task) || (value && !m_settings.wireless_report_task))
    {
        m_wPrintTaskID = m_taskExecutor.spawnTask("Send report", [this]()
        {
            this->sendReport();
        }, 
        1, 10000);
        return;
    }
    else if(!value && m_settings.wireless_report_task)
    {
        m_taskExecutor.stopTask(m_wPrintTaskID);
    }
}
