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

    const std::array<uint8_t, 512>& read()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_buffer;
    }

    std::array<uint8_t, 512>& getBuffer()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_buffer;
    }
};