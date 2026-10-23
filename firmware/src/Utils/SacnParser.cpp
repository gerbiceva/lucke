#include "SacnParser.h"
#include <algorithm>
#include <span>

#define ADDR_SEQ 111
#define ADDR_UNI_HI 113
#define ADDR_UNI_LO 114
#define ADDR_DMX 126

namespace Utils
{

    SacnParser::SacnParser() {}
	
	SacnParser::SacnParser(uint8_t universe)
		: m_ip(239, 255, 0, universe)
	{
		m_udp.beginMulticast(m_ip, 5568);	
        m_inited = true;
	}

	void SacnParser::begin(uint8_t universe)
	{
		m_universe = universe;
		m_ip = IPAddress(239, 255, 0, universe);
		m_udp.beginMulticast(m_ip, 5568);
        m_inited = true;
	}

    void SacnParser::stop()
    {
        m_udp.stop();
    }

	bool SacnParser::update()
	{
		if (!m_inited) return false;
		int size = m_udp.parsePacket();
		if (size <= 0) return false;

		// Drop packets that wouldn't fit the receive buffer or are too short
		// to carry a full universe. The next parsePacket() discards the rest.
		if (size > static_cast<int>(m_packet.size()) ||
			size < static_cast<int>(ADDR_DMX + Dmx::c_universeSize))
		{
			return false;
		}

		m_udp.read(m_packet.data(), size);

		uint16_t packetUni = (m_packet[ADDR_UNI_HI] << 8) | m_packet[ADDR_UNI_LO];
		if (packetUni != m_universe) return false;

		std::ranges::copy(std::span{m_packet}.subspan<ADDR_DMX, Dmx::c_universeSize>(), m_buffer.begin());
		m_seq_prev = m_seq;
		m_seq = m_packet[ADDR_SEQ];
		return true;
	}

	bool SacnParser::dmx(Dmx::UniverseView out)
	{
        if(update())
        {
            std::ranges::copy(m_buffer, out.begin());
        	return true;
        }

    	return false;
	}

    uint8_t SacnParser::getSeq()
    {
        return m_seq;
    }

	uint8_t SacnParser::getSeqDiff() const
	{
		return static_cast<uint8_t>(m_seq - m_seq_prev);
	}

}