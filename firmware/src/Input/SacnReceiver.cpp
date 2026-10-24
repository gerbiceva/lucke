#include "SacnReceiver.h"
#include <algorithm>
#include <unistd.h>
#include "lwip/sockets.h"

namespace
{
    constexpr uint16_t c_port = 5568;

    // Byte offsets into an E1.31 (sACN) data packet.
    constexpr size_t ADDR_ACN_ID = 4;
    constexpr size_t ADDR_ROOT_VECTOR = 18;
    constexpr size_t ADDR_FRAMING_VECTOR = 40;
    constexpr size_t ADDR_PRIORITY = 108;
    constexpr size_t ADDR_SEQ = 111;
    constexpr size_t ADDR_OPTIONS = 112;
    constexpr size_t ADDR_UNI = 113;
    constexpr size_t ADDR_DMP_VECTOR = 117;
    constexpr size_t ADDR_COUNT = 123;        // start code + slots
    constexpr size_t ADDR_START_CODE = 125;
    constexpr size_t ADDR_DMX = 126;

    constexpr std::array<uint8_t, 12> c_acnId{'A', 'S', 'C', '-', 'E', '1', '.', '1', '7', 0, 0, 0};
    constexpr uint32_t c_vectorRootData = 0x04;
    constexpr uint32_t c_vectorFramingData = 0x02;
    constexpr uint8_t c_vectorDmpSetProperty = 0x02;
    constexpr uint8_t c_optionPreview = 0x80;
    constexpr uint8_t c_optionTerminated = 0x40;
    // 0x00 carries levels. Others (0xDD = per-channel priority) share the
    // universe and must not be shown as levels.
    constexpr uint8_t c_startCodeDmx = 0x00;

    uint16_t be16(std::span<const uint8_t> p, size_t at)
    {
        return static_cast<uint16_t>((p[at] << 8) | p[at + 1]);
    }

    uint32_t be32(std::span<const uint8_t> p, size_t at)
    {
        return (static_cast<uint32_t>(be16(p, at)) << 16) | be16(p, at + 2);
    }
}

namespace Input
{
    SacnReceiver::~SacnReceiver()
    {
        close();
    }

    bool SacnReceiver::open()
    {
        m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_sock < 0) return false;

        int yes = 1;
        setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(c_port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            close();
            return false;
        }
        return true;
    }

    void SacnReceiver::close()
    {
        if (m_sock >= 0) ::close(m_sock);
        m_sock = -1;
    }

    bool SacnReceiver::isOpen() const
    {
        return m_sock >= 0;
    }

    bool SacnReceiver::join(uint16_t universe)
    {
        return membership(universe, IP_ADD_MEMBERSHIP);
    }

    void SacnReceiver::leave(uint16_t universe)
    {
        membership(universe, IP_DROP_MEMBERSHIP);
    }

    bool SacnReceiver::membership(uint16_t universe, int option)
    {
        if (m_sock < 0) return false;

        ip_mreq mreq{};
        mreq.imr_multiaddr.s_addr = htonl(0xEFFF0000u | universe);   // 239.255.hi.lo
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        return setsockopt(m_sock, IPPROTO_IP, option, &mreq, sizeof(mreq)) == 0;
    }

    std::optional<SacnReceiver::Packet> SacnReceiver::receive(uint32_t timeoutMs)
    {
        while (m_sock >= 0)
        {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(m_sock, &fds);
            timeval tv{};
            tv.tv_sec = timeoutMs / 1000;
            tv.tv_usec = (timeoutMs % 1000) * 1000;
            if (select(m_sock + 1, &fds, nullptr, nullptr, &tv) <= 0) return std::nullopt;

            // A larger datagram is truncated to the buffer; it isn't E1.31
            // data, so parse() rejects it.
            ssize_t n = recv(m_sock, m_packet.data(), m_packet.size(), 0);
            if (n <= 0) return std::nullopt;

            if (auto pkt = parse(static_cast<size_t>(n))) return pkt;
            timeoutMs = 0;   // skip junk without waiting again
        }
        return std::nullopt;
    }

    std::optional<SacnReceiver::Packet> SacnReceiver::parse(size_t size) const
    {
        std::span<const uint8_t> p{m_packet.data(), size};
        if (size < ADDR_DMX) return std::nullopt;

        if (!std::ranges::equal(p.subspan(ADDR_ACN_ID, c_acnId.size()), c_acnId)) return std::nullopt;
        if (be32(p, ADDR_ROOT_VECTOR) != c_vectorRootData) return std::nullopt;
        if (be32(p, ADDR_FRAMING_VECTOR) != c_vectorFramingData) return std::nullopt;
        if (p[ADDR_DMP_VECTOR] != c_vectorDmpSetProperty) return std::nullopt;
        if (p[ADDR_OPTIONS] & (c_optionPreview | c_optionTerminated)) return std::nullopt;
        if (p[ADDR_START_CODE] != c_startCodeDmx) return std::nullopt;

        // Senders may transmit fewer than 512 slots.
        uint16_t count = be16(p, ADDR_COUNT);
        if (count < 1 || ADDR_START_CODE + count > size) return std::nullopt;

        return Packet{be16(p, ADDR_UNI), p[ADDR_SEQ], p[ADDR_PRIORITY], p.subspan(ADDR_DMX, count - 1)};
    }
}
