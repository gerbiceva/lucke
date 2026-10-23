#pragma once
#include <WiFi.h>
#include <array>
#include "Utils/Dmx.h"

namespace Utils 
{

class SacnParser
{
	IPAddress m_ip;
	WiFiUDP m_udp;
    std::array<uint8_t, 1500> m_packet{};

    uint8_t m_seq = 0U;
	uint8_t m_seq_prev = 0U;

	Dmx::Universe m_buffer{};
    uint8_t m_universe = 0;
    bool m_inited = false;

	bool update();
public:
	SacnParser();
	SacnParser(uint8_t universe);

	void begin(uint8_t universe);
    void stop();

	bool dmx(Dmx::UniverseView out);
    uint8_t getSeq();
	uint8_t getSeqDiff() const;
};

}