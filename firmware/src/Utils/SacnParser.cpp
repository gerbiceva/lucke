#include "SacnParser.h"

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
		if(size)
		{
			m_udp.read(packet, size);

			uint16_t packetUni = (packet[ADDR_UNI_HI] << 8) | packet[ADDR_UNI_LO];
			if (packetUni != m_universe) return false;

			memcpy(m_buffer, packet + ADDR_DMX, 512);
			m_seq_prev = m_seq;
            m_seq = packet[ADDR_SEQ];
			return true;
		}

		return false;
	}

	bool SacnParser::dmx(uint8_t* copyBuffer)
	{
        if(update())
        {
            memcpy(copyBuffer, m_buffer, 512);
			m_sequenceDiff.push(getSeqDiff());
			if(m_sequenceDiff.size() > 500)
			{
				m_sequenceDiff.pop();
			}

            // Utils::Logger::printf("DMX seq:%d | %d %d %d %d %d %d %d %d %d\n",
            //     m_seq,
            //     copyBuffer[0], copyBuffer[1], copyBuffer[2],
            //     copyBuffer[3], copyBuffer[4], copyBuffer[5],
            //     copyBuffer[6], copyBuffer[7], copyBuffer[8]);

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

	std::vector<uint8_t> SacnParser::getSeqDiffs()
	{
		std::vector<uint8_t> vec;
		// vec.resize(m_sequenceDiff.size());

		// int i = 0;
		for(; !m_sequenceDiff.empty(); m_sequenceDiff.pop())
		{
			// vec[i++]
			vec.push_back(m_sequenceDiff.front());
		}

		return vec;
	}

}