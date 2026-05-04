#pragma once
#include <map>
#include "sACN.h"
#include <WiFi.h>
#include "esp_wifi.h"

namespace Handler 
{
struct Sacn
{
	static std::map<uint8_t, uint8_t> activeReceivers;
	static std::map<uint8_t, Receiver*> sacnMap;
	static std::map<uint8_t, std::array<uint8_t, 512U>> bufferMap;
	static WiFiUDP udp;

	static void incrementActive(uint8_t universe);
	static void decrementActive(uint8_t universe);

    static uint8_t* buffer(uint8_t universe);
	static void update();

};
}
