#include <FastLED.h>
#include "WiFi.h"
#include "sACN.h"
#include <ArduinoJson.h>

// LED shit
#define NUM_LEDS 100
#define DATA_PIN 5
#define UNIVERSE 4
CLEDController *cled;
CRGB leds[NUM_LEDS];
uint8_t cbuffer[512] = {};

// Network shit
uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0x48}; // MAC Adress of your device
WiFiUDP udp;
Receiver recv(udp); // universe 1
// const char *ssid = "nLa";
// const char *password = "tugicamalo";
const char *ssid = "Smart Toilet";
const char *password = "bbbbbbbbb";

int lastDMXFramerate = 0;

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);    // Set your gateway
// IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask

// Misc

void dmxReceived()
{
  recv.dmx(cbuffer);

  // cled->setLeds((CRGB*)cbuffer, NUM_LEDS);
  // for (int i = 0; i < 100; i++) {
  //   leds[i] = CRGB(buffer[3 * i], buffer[3 * i + 1], buffer[3 * i + 2]);
  // }
  // leds[0] = CRGB(buffer[0], buffer[1], buffer[2]);
  // Serial.print("Led1:");
  // Serial.print(buffer[0]);
  // Serial.print(buffer[1]);
  // Serial.println(buffer[2]);
}

void newSource()
{
  // Serial.print("new soure name: ");
  Serial.println(recv.name());
}

void framerate()
{
  Serial.print("Framerate fps: ");
  Serial.println(recv.framerate());
  lastDMXFramerate = recv.framerate();
}

void timeOut()
{
  // Serial.println("Timeout!");
}
// bool odd;

void dmxLoop(void *)
{
  while (true)
  {
    recv.update();
    vTaskDelay(1);
  }
}

void ledLoop(void *)
{
  while (true)
  {
    FastLED.show();
    vTaskDelay(1);
  }
}

void statReportLoop(void *)
{
  while (true)
  {
    JsonDocument doc;
    doc["universe"] = UNIVERSE;
    doc["heap_size"] = ESP.getHeapSize();
    doc["heap_free"] = ESP.getFreeHeap();
    doc["local_ip"] = WiFi.localIP();
    doc["ssid"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["last_DMX_framerate"] = lastDMXFramerate;

    doc["first_5_leds"] = JsonDocument();
    // Create a nested JsonArray in the JSON document
    JsonArray jsonArray = doc["first_5_leds"].to<JsonArray>();
    // Add the elements from first5Buff into the JSON array
    for (int i = 0; i < 15; i++)
    {
      jsonArray.add(cbuffer[i]);
    }

    udp.beginPacket(WiFi.broadcastIP(), 12345);

    serializeJson(doc, udp);
    // udp.printf("heap %d, cycle: %d, chip cores: %d, PSram: %d, CPU Freq %d, heapsize: %d, maxHeap: %d, maxPSram: %d", ESP.getFreeHeap(), ESP.getCycleCount(), ESP.getChipCores(), ESP.getFreePsram(), ESP.getCpuFreqMHz(), ESP.getHeapSize(), ESP.getMinFreeHeap(), ESP.getMinFreePsram());
    udp.endPacket();
    vTaskDelay(1000);
  }
}

void playIdleAnimation(void *)
{
  while (true)
  {
    cbuffer[((millis() / 10) % (NUM_LEDS * 3))] = 255;
    cbuffer[((millis() / 10) % (NUM_LEDS * 3)) - 1] = 0;
    vTaskDelay(5);
  }
}

void checkNetwork(void *)
{
  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      TaskHandle_t animation = NULL;
      Serial.println("Lost connection");
      xTaskCreate(
          playIdleAnimation, // Task function
          "Animation",       // Name of the task (for debugging)
          500,               // Stack size in words
          NULL,              // Parameter passed to the task
          2,                 // Task priority
          &animation         // Handle to the task
      );
      while (WiFi.status() != WL_CONNECTED)
      {
        // WiFi.begin(ssid, password);
        vTaskDelay(1000);
      }
      vTaskDelete(animation);
    }
    vTaskDelay(5);
  }
}

void setup()
{
  Serial.begin(9600);
  // WiFi.config(local_IP, gateway, subnet);
  // vTaskDelay(1000);
  WiFi.useStaticBuffers(1);
  WiFi.setScanMethod(WIFI_FAST_SCAN);
  WiFi.mode(WIFI_STA);
  // esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT40);
  WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   vTaskDelay(50);              // Wait for 500ms before checking again
  //   Serial.print(".");       // Print a dot to show progress
  // }

  // esp_bluedroid_disable();
  // esp_bluedroid_deinit();
  // esp_bt_controller_disable();
  // esp_bt_controller_deinit();
  recv.callbackDMX(dmxReceived);
  recv.callbackSource(newSource);
  recv.callbackFramerate(framerate);
  recv.callbackTimeout(timeOut);
  recv.begin(UNIVERSE, true);
  // Serial.println("sACN start");
  // Serial.println(WiFi.locBSSIDalIP());
  // Serial.println(portNUM_PROCESSORS);
  cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>((CRGB *)cbuffer, NUM_LEDS);
  // cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>(leds, NUM_LEDS);
  // randomSeed(analogRead(0));
  // odd = true;
  xTaskCreate(dmxLoop, "DMX", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(ledLoop, "LED", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(checkNetwork, "Wifi check", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(statReportLoop, "Logging", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

void loop()
{
  // recv.update();
  // FastLED.show();
  // Serial.println(WiFi.localIP());
  // if (WiFi.isConnected()) {
  // Serial.println("connected");
  // Serial.println(WiFi.localIP());
  // }
  // recv.update();
  // for (int i = 0; i < 100; i++) {
  //   if (odd) {
  //     leds[i] = CRGB::Black;
  //   } else {
  //     leds[i] = CRGB(random(0, 156), random(0, 156), random(0, 156));
  //   }
  // }
  // FastLED.show();
  // vTaskDelay(50);
  // odd = !odd;
  vTaskDelete(NULL);
}
