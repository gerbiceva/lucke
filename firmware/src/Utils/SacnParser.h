#pragma once
#include <WiFi.h>

namespace Utils 
{

class SacnParser
{
	IPAddress m_ip;
	WiFiUDP m_udp;
    uint8_t packet[1500];

    uint8_t m_seq;
	uint8_t m_seq_prev = 0U;
	uint8_t m_buffer[512];

    bool m_inited = false;

	bool update();
public:
	SacnParser();
	SacnParser(uint8_t universe);

	void begin(uint8_t universe);
    void stop();

	void dmx(uint8_t* copyBuffer);
    uint8_t getSeq();
	uint8_t getSeqDiff() const;
};

}