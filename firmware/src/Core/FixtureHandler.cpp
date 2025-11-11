#include "FixtureHandler.h"
#include "Utils/Wifi.h"
#include "Core/Fixture.h"
#include "Output/HardwareLED.h"

std::vector<Fixture*> FixtureHandler::fixtures;
std::atomic<bool> FixtureHandler::connected;

void FixtureHandler::animateConnecting(void*)
{
    // uint16_t off = 0;
    // get(0)->getSrcBuffer()
    // astera60.getSrcBuffer()[off] = 255;
    // astera60.getSrcBuffer()[off == 0 ? 179 : off - 1] = 0;
    // off = (off + 1) % 180;
    // astera60.update();
    // Output::updateFastLED();
}

void FixtureHandler::checkNetwork(void *) {
    while (true) {
        // if not connected
        if (WiFi.status() != WL_CONNECTED) {
            connected = false;

            // TaskHandle_t animation = NULL;
            // xTaskCreate(
            //     Controller::playIdleAnimation, 		// Task function
            //     "Animation",						// Name of the task (for debugging)
            //     2000,								// Stack size in words
            //     NULL,								// Parameter passed to the task
            //     1,									// Task priority
            //     &animation							// Handle to the task
            // );

            // while not connected
            while (WiFi.status() != WL_CONNECTED) {
                vTaskDelay(10);
            }

            connected = true;
            // vTaskDelete(animation);
            // Controller::get().clear();
        }

        vTaskDelay(50);
    }
}

void FixtureHandler::updateTask(void*)
{
    while(true)
    {
        if(connected)
        {
            Input::Handler::update();
        
            for(Fixture* fix : fixtures)
            {
                fix->update();
            }
        }
        
        Output::updateFastLED();
        vTaskDelay(20);
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