#pragma once
#include <Arduino.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdarg>
#include <optional>
#include <span>
#include <string_view>

namespace Utils
{
    enum class Level : uint8_t
    {
        Debug,
        Info,
        Warn,
        Error
    };

    // Recent log lines, numbered; oldest dropped first.
    class LogHistory
    {
    public:
        static constexpr size_t Capacity = 4096;
        static constexpr size_t MaxLine = 255;  // fits the 1-byte length prefix

        void append(std::string_view line);

        // First line numbered >= seq; seq is set to its number.
        std::optional<std::string_view> read(uint32_t& seq, std::span<char> out) const;

        uint32_t nextSeq() const;

    private:
        std::array<uint8_t, Capacity> m_buf{};
        uint32_t m_head = 0;      // where the next line is written
        uint32_t m_tail = 0;      // oldest stored line
        uint32_t m_tailSeq = 0;   // number of the line at m_tail
        uint32_t m_nextSeq = 0;
        // No lock member: its initializer isn't zero, which would move the
        // whole buffer from .bss to .data (4 KB of flash). See Logger.cpp.
    };

    class Log
    {
    public:
        constexpr explicit Log(const char* tag) : m_tag(tag) {}

        // printf-style; -Werror=format checks the arguments.
        [[gnu::format(printf, 2, 3)]] void debug(const char* fmt, ...) const;
        [[gnu::format(printf, 2, 3)]] void info(const char* fmt, ...) const;
        [[gnu::format(printf, 2, 3)]] void warn(const char* fmt, ...) const;
        [[gnu::format(printf, 2, 3)]] void error(const char* fmt, ...) const;

        static void begin(uint32_t baudrate = 9600);

        static void setLevel(Level level) { s_level.store(level, std::memory_order_relaxed); }
        static Level level() { return s_level.load(std::memory_order_relaxed); }
        static bool enabled(Level level) { return level >= Log::level(); }

        static const LogHistory& history() { return s_history; }

        // Unprefixed serial output, not kept in history. Never blocks.
        static void raw(std::string_view text);

    private:
        static constexpr Level HistoryLevel = Level::Info;

        void write(Level level, const char* fmt, va_list args) const;

        const char* m_tag;

        static std::atomic<Level> s_level;
        static std::atomic<bool> s_serialReady;
        static std::atomic<uint32_t> s_dropped;  // serial lines skipped since the last write
        static constinit LogHistory s_history;
    };
}
