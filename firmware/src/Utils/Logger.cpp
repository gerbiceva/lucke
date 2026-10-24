#include "Logger.h"
#include <algorithm>
#include <cstring>

namespace
{
    // Output iterator into a fixed buffer. Writes past the end are dropped
    // but still counted, so the caller can tell the text was cut.
    struct BoundedOut
    {
        using difference_type = std::ptrdiff_t;

        char* buf = nullptr;
        size_t cap = 0;
        size_t count = 0;

        BoundedOut& operator=(char c)
        {
            if (count < cap) buf[count] = c;
            ++count;
            return *this;
        }
        BoundedOut& operator*() { return *this; }
        BoundedOut& operator++() { return *this; }
        BoundedOut& operator++(int) { return *this; }

        size_t size() const { return std::min(count, cap); }
        bool truncated() const { return count > cap; }
    };

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

        portENTER_CRITICAL(&m_lock);
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
        portEXIT_CRITICAL(&m_lock);
    }

    std::optional<std::string_view> LogHistory::read(uint32_t& seq, std::span<char> out) const
    {
        portENTER_CRITICAL(&m_lock);
        if (seq < m_tailSeq) seq = m_tailSeq;
        if (seq >= m_nextSeq)
        {
            portEXIT_CRITICAL(&m_lock);
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
        portEXIT_CRITICAL(&m_lock);

        return std::string_view(out.data(), len);
    }

    uint32_t LogHistory::nextSeq() const
    {
        portENTER_CRITICAL(&m_lock);
        uint32_t seq = m_nextSeq;
        portEXIT_CRITICAL(&m_lock);
        return seq;
    }

    void Log::begin(uint32_t baudrate)
    {
        if (s_serialReady.load()) return;
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
        // USB serial (HWCDC). When the cable is in but nothing on the host
        // reads, each write retries 20x the TX timeout (100 ms default)
        // before giving up. raw() checks for room first so that path is
        // rare; a short timeout bounds it when two tasks race for the last
        // space. The buffer fits a whole report or reply. Both must be set
        // before begin().
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
        // No host on the cable: nobody to show it to, the history has it.
        if (!Serial.isPlugged()) return;
#endif
        // A host that stopped reading leaves the TX buffer full; skip
        // instead of waiting for room, and say so once it drains.
        if (Serial.availableForWrite() < static_cast<int>(text.size()))
        {
            s_dropped.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        if (uint32_t dropped = s_dropped.exchange(0, std::memory_order_relaxed))
        {
            char note[48];
            BoundedOut out{note, sizeof(note)};
            out = std::format_to(out, "-- {} serial lines dropped --\r\n", dropped);
            Serial.write(reinterpret_cast<const uint8_t*>(note), out.size());
        }
        Serial.write(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    }

    void Log::write(Level level, std::string_view fmt, std::format_args args) const
    {
        // "I   12.345 [WIFI] message", cut at MaxLine, then "\r\n".
        char line[LogHistory::MaxLine + 2];
        BoundedOut out{line, LogHistory::MaxLine};
        const uint32_t ms = millis();
        out = std::format_to(out, "{} {:>4}.{:03} [{}] ", letter(level), ms / 1000, ms % 1000, m_tag);
        out = std::vformat_to(out, fmt, args);

        size_t len = out.size();
        if (out.truncated()) std::memcpy(line + len - 3, "...", 3);

        if (level >= HistoryLevel) s_history.append({line, len});

        line[len++] = '\r';
        line[len++] = '\n';
        raw({line, len});
    }
}
