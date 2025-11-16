#include "FixtureHandler.h"
#include "Utils/Wifi.h"
#include "Fixture/Fixture.h"
#include "Output/HardwareLED.h"

std::vector<Fixture*> FixtureHandler::fixtures;

void FixtureHandler::updateTask(void*)
{
    for(Fixture* fix : fixtures)
    {
        fix->update();
    }
        
}

void FixtureHandler::wirelessConfig(void*)
{
    // WiFiServer server(8888);
    // server.begin();
    
    // while(true) {
    //     // Serial.println(WiFi.localIP());
    //     WiFiClient client = server.available();
    //     if (client) {
    //         while (client.connected()) {
    //             if (client.available()) {
    //             }
    //         }
    //     }
    // }
}