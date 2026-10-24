#pragma once
#include <algorithm>
#include <mutex>
#include "Utils/Dmx.h"


class SyncBuffer
{
    Dmx::Universe m_buffer{};
    std::mutex m_mutex;
public:
    void write(Dmx::UniverseCView src)
    {
        std::scoped_lock lock(m_mutex);
        std::ranges::copy(src, m_buffer.begin());
    }

    // Copies the buffer into the caller-owned destination while holding the
    // lock. Returning a reference would release the lock before the caller
    // reads, reopening the torn-read race this buffer exists to prevent.
    void read(Dmx::UniverseView dst)
    {
        std::scoped_lock lock(m_mutex);
        std::ranges::copy(m_buffer, dst.begin());
    }
};
