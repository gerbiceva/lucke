#include "Logger.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace
{
    char letter(Utils::Level level)
    {
        switch (level)
        {
            case Utils::Level::Debug: return 'D';
            case Utils::Level::Info:  return 'I';
            case Utils::Level::Warn:  return 'W';
            case Utils::Level::Error: return 'E';
        }
        return '?';
    }

    // LogHistory's lock. Kept out of the class: its initializer isn't zero,
    // which would put the whole 4 KB buffer in .data instead of .bss.
    portMUX_TYPE s_historyLock = portMUX_INITIALIZER_UNLOCKED;
}

namespace Utils
{
    std::atomic<Level> Log::s_level{Level::Info};
    std::atomic<bool> Log::s_serialReady{false};
    std::atomic<uint32_t> Log::s_dropped{0};
    constinit LogHistory Log::s_history;

    void LogHistory::append(std::string_view line)
    {
        const size_t len = std::min(line.size(), MaxLine);
        const uint32_t need = len + 1;

        portENTER_CRITICAL(&s_historyLock);
        while (m_head - m_tail + need > Capacity)
        {
            m_tail += 1 + m_buf[m_tail % Capacity];
            ++m_tailSeq;
        }
        m_buf[m_head++ % Capacity] = static_cast<uint8_t>(len);
        for (size_t i = 0; i < len; i++)
        {
            m_buf[m_head++ % Capacity] = static_cast<uint8_t>(line[i]);
        }
        ++m_nextSeq;
        portEXIT_CRITICAL(&s_historyLock);
    }

    std::optional<std::string_view> LogHistory::read(uint32_t& seq, std::span<char> out) const
    {
        portENTER_CRITICAL(&s_historyLock);
        if (seq < m_tailSeq) seq = m_tailSeq;
        if (seq >= m_nextSeq)
        {
            portEXIT_CRITICAL(&s_historyLock);
            return std::nullopt;
        }

        uint32_t pos = m_tail;
        for (uint32_t s = m_tailSeq; s < seq; s++)
        {
            pos += 1 + m_buf[pos % Capacity];
        }
        const size_t len = std::min<size_t>(m_buf[pos++ % Capacity], out.size());
        for (size_t i = 0; i < len; i++)
        {
            out[i] = static_cast<char>(m_buf[pos++ % Capacity]);
        }
        portEXIT_CRITICAL(&s_historyLock);

        return std::string_view(out.data(), len);
    }

    uint32_t LogHistory::nextSeq() const
    {
        portENTER_CRITICAL(&s_historyLock);
        uint32_t seq = m_nextSeq;
        portEXIT_CRITICAL(&s_historyLock);
        return seq;
    }

    void Log::begin(uint32_t baudrate)
    {
        if (s_serialReady.load()) return;
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
        // Short TX timeout so a stalled host can't block a task. Set before begin().
        Serial.setTxBufferSize(4096);
        Serial.setTxTimeoutMs(2);
#endif
        Serial.begin(baudrate);
        s_serialReady.store(true);
    }

    void Log::raw(std::string_view text)
    {
        if (!s_serialReady.load(std::memory_order_relaxed)) return;
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
        if (!Serial.isPlugged()) return;
#endif
        // Host not reading: drop instead of waiting.
        if (Serial.availableForWrite() < static_cast<int>(text.size()))
        {
            s_dropped.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        if (uint32_t dropped = s_dropped.exchange(0, std::memory_order_relaxed))
        {
            char note[48];
            int n = snprintf(note, sizeof(note), "-- %lu serial lines dropped --\r\n", static_cast<unsigned long>(dropped));
            Serial.write(reinterpret_cast<const uint8_t*>(note), std::min<size_t>(n, sizeof(note) - 1));
        }
        Serial.write(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    }

    void Log::write(Level level, const char* fmt, va_list args) const
    {
        // "I   12.345 [WIFI] message\r\n". +1 for vsnprintf's NUL, +2 for "\r\n".
        constexpr size_t cap = LogHistory::MaxLine;
        char line[cap + 3];
        const uint32_t ms = millis();
        int n = snprintf(line, cap + 1, "%c %4lu.%03lu [%s] ", letter(level),
                         static_cast<unsigned long>(ms / 1000), static_cast<unsigned long>(ms % 1000), m_tag);
        size_t len = std::min<size_t>(n > 0 ? n : 0, cap);
        n = vsnprintf(line + len, cap + 1 - len, fmt, args);
        size_t total = len + (n > 0 ? n : 0);
        len = std::min(total, cap);
        if (total > cap) std::memcpy(line + len - 3, "...", 3);

        if (level >= HistoryLevel) s_history.append({line, len});

        line[len++] = '\r';
        line[len++] = '\n';
        raw({line, len});
    }

    void Log::debug(const char* fmt, ...) const
    {
        if (!enabled(Level::Debug)) return;
        va_list args;
        va_start(args, fmt);
        write(Level::Debug, fmt, args);
        va_end(args);
    }

    void Log::info(const char* fmt, ...) const
    {
        if (!enabled(Level::Info)) return;
        va_list args;
        va_start(args, fmt);
        write(Level::Info, fmt, args);
        va_end(args);
    }

    void Log::warn(const char* fmt, ...) const
    {
        if (!enabled(Level::Warn)) return;
        va_list args;
        va_start(args, fmt);
        write(Level::Warn, fmt, args);
        va_end(args);
    }

    void Log::error(const char* fmt, ...) const
    {
        if (!enabled(Level::Error)) return;
        va_list args;
        va_start(args, fmt);
        write(Level::Error, fmt, args);
        va_end(args);
    }
}
