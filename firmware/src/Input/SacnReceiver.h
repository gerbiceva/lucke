#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace Input
{
    // One UDP socket on the sACN port for every subscribed universe. join()
    // adds a universe's multicast group to it; the universe field in each
    // packet decides where the data goes. Unicast sACN to the device's IP
    // arrives on the same socket.
    //
    // open/close/receive belong to one task (the input task). join/leave may
    // be called from another, as long as the caller serializes them against
    // open/close.
    class SacnReceiver
    {
    public:
        static constexpr uint16_t c_minUniverse = 1;
        static constexpr uint16_t c_maxUniverse = 63999;

        struct Packet
        {
            uint16_t universe;
            uint8_t sequence;
            uint8_t priority;
            // At most 512 slots; senders may send fewer. Points into the
            // receive buffer: valid until the next receive().
            std::span<const uint8_t> slots;
        };

        SacnReceiver() = default;
        ~SacnReceiver();

        // Owns a socket handle; a copy would close it twice.
        SacnReceiver(const SacnReceiver&) = delete;
        SacnReceiver& operator=(const SacnReceiver&) = delete;

        bool open();
        void close();
        [[nodiscard]] bool isOpen() const;

        // Fail (harmlessly) while the socket is closed; join again after open().
        bool join(uint16_t universe);
        void leave(uint16_t universe);

        // Next valid DMX packet. Waits up to timeoutMs for traffic; 0 only
        // drains what is already queued. nullopt = nothing (more) to read.
        std::optional<Packet> receive(uint32_t timeoutMs);

    private:
        bool membership(uint16_t universe, int option);
        std::optional<Packet> parse(size_t size) const;

        int m_sock = -1;
        std::array<uint8_t, 638> m_packet{};   // largest E1.31 data packet
    };
}
