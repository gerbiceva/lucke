#pragma once
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <mutex>
#include "Utils/Dmx.h"


class SyncBuffer
{
    Dmx::Universe m_buffer{};
    std::mutex m_mutex;
    // Bumped by every write that changed the data. Atomic so the render task
    // can check for news without taking the lock or copying.
    std::atomic<uint32_t> m_version{0};
public:
    // Identical data (a sender repeating a static frame) is not a change.
    void write(Dmx::UniverseCView src)
    {
        std::scoped_lock lock(m_mutex);
        if (std::ranges::equal(src, m_buffer)) return;
        std::ranges::copy(src, m_buffer.begin());
        m_version.fetch_add(1, std::memory_order_relaxed);
    }

    [[nodiscard]] uint32_t version() const
    {
        return m_version.load(std::memory_order_relaxed);
    }

    // Copies the buffer into the caller-owned destination while holding the
    // lock. Returning a reference would release the lock before the caller
    // reads, reopening the torn-read race this buffer exists to prevent.
    // Returns the version of the copied data.
    uint32_t read(Dmx::UniverseView dst)
    {
        std::scoped_lock lock(m_mutex);
        std::ranges::copy(m_buffer, dst.begin());
        return m_version.load(std::memory_order_relaxed);
    }
};
