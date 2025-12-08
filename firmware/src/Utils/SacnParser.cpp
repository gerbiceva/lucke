#include "SacnParser.h"

#define ADDR_SEQ 111
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
        // if(!m_inited)
        // {}
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
		int size = m_udp.parsePacket();
		if(size)
		{
			m_udp.read(packet, size);

			memcpy(m_buffer, packet + ADDR_DMX, 512);
            m_seq = packet[ADDR_SEQ];
			return true;
		}

		return false;
	}

	void SacnParser::dmx(uint8_t* copyBuffer)
	{
        if(update())
        {
            memcpy(copyBuffer, m_buffer, 512);
        }
	}

    uint8_t SacnParser::getSeq()
    {
        return m_seq;
    }

}