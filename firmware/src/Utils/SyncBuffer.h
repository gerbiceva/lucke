#pragma once
#include <mutex>
#include <array>
#include <string.h>


class SyncBuffer
{
    std::array<uint8_t, 512> m_buffer;
    std::mutex m_mutex;
public:
    void write(const uint8_t* buff)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        memcpy(m_buffer.data(), buff, 512);
    }

    // Copies the buffer into the caller-owned destination while holding the
    // lock. Returning a reference would release the lock before the caller
    // reads, reopening the torn-read race this buffer exists to prevent.
    void read(uint8_t* dst)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        memcpy(dst, m_buffer.data(), 512);
    }
};