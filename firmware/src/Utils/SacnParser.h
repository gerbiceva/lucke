#pragma once
#include <WiFi.h>

namespace Utils 
{

class SacnParser
{
	IPAddress m_ip;
	WiFiUDP m_udp;
    uint8_t packet[1500];


    bool m_inited = false;
	uint8_t m_buffer[512];
	const uint16_t m_dataOffset = 126U;
	// uint8_t inter

	bool update();
public:
	SacnParser();
	SacnParser(uint8_t universe);

	void begin(uint8_t universe);
    void stop();

	void dmx(uint8_t* copyBuffer);

};

}