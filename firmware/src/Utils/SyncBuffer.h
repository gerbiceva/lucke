#pragma once
#include <mutex>
#include <array>
#include <string.h>


class SyncBuffer
{
    std::array<uint8_t, 512> m_buffer;
    std::mutex m_mutex;
public:
    void write(uint8_t* buff)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        memcpy(&m_buffer[0], buff, 512);
    }

    // Copy the buffer into the caller's destination while holding the lock.
    // Returning a reference instead would release the lock before the caller
    // reads the data, defeating the synchronisation and allowing torn reads.
    void read(uint8_t* dst)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        memcpy(dst, m_buffer.data(), 512);
    }
};