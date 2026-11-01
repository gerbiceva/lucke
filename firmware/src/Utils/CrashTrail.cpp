#include "CrashTrail.h"
#include <Arduino.h>
#include <esp_attr.h>
#include <algorithm>
#include <atomic>
#include <cstring>

namespace
{
    using Utils::CrashTrail::Capacity;

    constexpr uint32_t c_magic = 0x4b43554c;  // "LUCK"

    struct Trail
    {
        uint32_t magic;
        uint32_t head;        // where the next byte goes
        uint32_t wrapped;     // buf has been filled at least once
        uint32_t uptimeMs;
        char bootId[12];
        char version[20];
        char buf[Capacity];   // '\n'-separated lines
    };

    // Not cleared by a reset; garbage after a power-on, hence the magic.
    RTC_NOINIT_ATTR Trail s_trail;
    // What the previous boot left, copied before this boot writes over it.
    Trail s_previous;
    std::atomic<bool> s_taken{false};
    portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;

    // Caller holds s_lock.
    void takeOver()
    {
        if (s_taken.load(std::memory_order_relaxed)) return;
        std::memcpy(&s_previous, &s_trail, sizeof(Trail));
        std::memset(&s_trail, 0, sizeof(Trail));
        s_trail.magic = c_magic;
        s_taken.store(true, std::memory_order_relaxed);
    }

    // Caller holds s_lock.
    void put(char c)
    {
        s_trail.buf[s_trail.head] = c;
        if (++s_trail.head == Capacity)
        {
            s_trail.head = 0;
            s_trail.wrapped = 1;
        }
    }

    template <size_t N>
    void setField(char (&dst)[N], std::string_view src)
    {
        const size_t len = std::min(src.size(), N - 1);
        std::memcpy(dst, src.data(), len);
        dst[len] = '\0';
    }

    template <size_t N>
    std::string getField(const char (&src)[N])
    {
        return std::string(src, strnlen(src, N));
    }
}

namespace Utils::CrashTrail
{
    void setBoot(std::string_view bootId, std::string_view version)
    {
        portENTER_CRITICAL(&s_lock);
        takeOver();
        setField(s_trail.bootId, bootId);
        setField(s_trail.version, version);
        portEXIT_CRITICAL(&s_lock);
    }

    void append(std::string_view line)
    {
        const size_t len = std::min(line.size(), Capacity - 1);

        portENTER_CRITICAL(&s_lock);
        takeOver();
        for (size_t i = 0; i < len; i++)
        {
            put(line[i] == '\n' ? ' ' : line[i]);
        }
        put('\n');
        s_trail.uptimeMs = millis();
        portEXIT_CRITICAL(&s_lock);
    }

    Previous previous()
    {
        portENTER_CRITICAL(&s_lock);
        takeOver();
        portEXIT_CRITICAL(&s_lock);

        // s_previous isn't written after takeOver(), so no lock from here.
        const Trail& t = s_previous;
        Previous prev;
        if (t.magic != c_magic || t.head >= Capacity) return prev;

        prev.valid = true;
        prev.bootId = getField(t.bootId);
        prev.version = getField(t.version);
        prev.uptimeMs = t.uptimeMs;

        // Oldest byte first. After a wrap the first line has lost its start,
        // so it is skipped.
        std::string text;
        if (t.wrapped) text.assign(t.buf + t.head, Capacity - t.head);
        text.append(t.buf, t.head);
        size_t start = 0;
        if (t.wrapped)
        {
            const size_t nl = text.find('\n');
            start = nl == std::string::npos ? text.size() : nl + 1;
        }

        while (start < text.size())
        {
            size_t end = text.find('\n', start);
            if (end == std::string::npos) end = text.size();
            std::string line = text.substr(start, end - start);
            // Nothing checks the bytes themselves; keep the result printable.
            for (char& c : line)
            {
                if (static_cast<unsigned char>(c) < 0x20 || c == 0x7f) c = '?';
            }
            if (!line.empty()) prev.lines.push_back(std::move(line));
            start = end + 1;
        }
        return prev;
    }
}
