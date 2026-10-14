#include "SacnSender.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

#include <string>
#include <format>
#include <cstring>

SacnSender::SacnSender()
    : m_sequence(0)
{}


SacnSender::SacnSender(uint8_t universe, const char* localIp)
    : m_universe(universe), m_sequence(0)
{
    begin(universe, localIp);
};

SacnSender::SacnSender(SacnSender&& other)
{
    m_universe = other.m_universe;
    m_sequence = other.m_sequence;
    m_socket = other.m_socket;
    m_destinationIp = other.m_destinationIp;
    m_inited = other.m_inited;
    other.m_socket = -1;
}

SacnSender::~SacnSender()
{
    close(m_socket);
}

void SacnSender::setBuffer(uint8_t* buffer)
{
    m_buffer = buffer;
}

void SacnSender::begin(uint8_t universe, const char* localIp)
{
    m_universe = universe;
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) 
    {
        std::cout << "Failed to create socket\n";
        return;
    }
    
    sockaddr_in local{};
    inet_pton(AF_INET, localIp, &local.sin_addr);
    if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_IF, &local, sizeof(local))) 
    {
        std::cout << "Set socket opt 1\n";
        return;
        // perror("setsockopt1");
    }
    
    unsigned char loop = 1;
    if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) 
    {
        std::cout << "Set socket opt 2\n";
        return;
        // perror("setsockopt");
    }
    
    m_destinationIp.sin_family = AF_INET;
    m_destinationIp.sin_port = htons(5568);

    std::string ip = std::format("239.255.0.{}", universe);
    inet_pton(AF_INET, ip.c_str(), &m_destinationIp.sin_addr);

    initializePacket();

    m_inited = true;
}


#include <iostream>
void SacnSender::send()
{
    if(!m_inited)
    {
        return;
    }

    m_packet[111] = m_sequence++;
    memcpy(&m_packet[126], m_buffer, 512);
    ssize_t sent = sendto(m_socket, m_packet, sizeof(m_packet), 0, (sockaddr*)&m_destinationIp, sizeof(m_destinationIp));
    // if (sent != sizeof(m_packet.)) 
    // {
    //     // perror("sendto");
    // }
}

void SacnSender::initializePacket()
{
     // m_packet.empty();

    m_packet[0] = 0x00; 
    m_packet[1] = 0x10;               
    m_packet[2] = 0x00; 
    m_packet[3] = 0x00;               
    
    memcpy(&m_packet[4], "ASC-E1.17\0\0\0", 12);        
    m_packet[16] = 0x70; 
    m_packet[17] = 0x64;            
    m_packet[18] = 0x00; 
    m_packet[19] = 0x00;            
    m_packet[38] = 0x00; 
    m_packet[39] = 0x01;            
    m_packet[113] = m_universe;  // Universe 1
    m_packet[125] = 0x02; // DMX start code
}
