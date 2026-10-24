#pragma once
#include <Arduino.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <format>
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

    // The most recent log lines, oldest dropped first, so the console can
    // show what happened before anyone was looking. Lines are stored whole
    // (length-prefixed) and numbered, so a reader can tell when lines it
    // hasn't seen yet were overwritten.
    class LogHistory
    {
    public:
        static constexpr size_t Capacity = 4096;
        static constexpr size_t MaxLine = 255;  // fits the 1-byte length prefix

        void append(std::string_view line);

        // Copies the first stored line numbered >= seq into out and sets seq
        // to that line's number. nullopt when there is no such line yet.
        std::optional<std::string_view> read(uint32_t& seq, std::span<char> out) const;

        // Number the next appended line will get.
        uint32_t nextSeq() const;

    private:
        // Positions only ever grow; Capacity divides 2^32, so % stays
        // consistent when they wrap.
        std::array<uint8_t, Capacity> m_buf{};
        uint32_t m_head = 0;      // where the next line is written
        uint32_t m_tail = 0;      // oldest stored line
        uint32_t m_tailSeq = 0;   // number of the line at m_tail
        uint32_t m_nextSeq = 0;
        mutable portMUX_TYPE m_lock = portMUX_INITIALIZER_UNLOCKED;
    };

    // One per module, holding its tag:
    //   namespace { constexpr Utils::Log s_log{"WIFI"}; }
    //   s_log.info("connecting to '{}'", ssid);
    //
    // A call below the current level costs one comparison. Otherwise the
    // line is formatted once, kept in the history (Info and up) and written
    // to serial without blocking: when no host is plugged in or the TX
    // buffer is full, the serial copy is skipped.
    class Log
    {
    public:
        constexpr explicit Log(const char* tag) : m_tag(tag) {}

        template<typename... Args>
        void debug(std::format_string<Args...> fmt, Args&&... args) const
        {
            if (enabled(Level::Debug)) write(Level::Debug, fmt.get(), std::make_format_args(args...));
        }

        template<typename... Args>
        void info(std::format_string<Args...> fmt, Args&&... args) const
        {
            if (enabled(Level::Info)) write(Level::Info, fmt.get(), std::make_format_args(args...));
        }

        template<typename... Args>
        void warn(std::format_string<Args...> fmt, Args&&... args) const
        {
            if (enabled(Level::Warn)) write(Level::Warn, fmt.get(), std::make_format_args(args...));
        }

        template<typename... Args>
        void error(std::format_string<Args...> fmt, Args&&... args) const
        {
            if (enabled(Level::Error)) write(Level::Error, fmt.get(), std::make_format_args(args...));
        }

        // Starts serial output. Lines logged before this are still kept in
        // the history.
        static void begin(uint32_t baudrate = 9600);

        static void setLevel(Level level) { s_level.store(level, std::memory_order_relaxed); }
        static Level level() { return s_level.load(std::memory_order_relaxed); }
        static bool enabled(Level level) { return level >= Log::level(); }

        static const LogHistory& history() { return s_history; }

        // Writes text to serial as is: no prefix, not kept in the history.
        // For console output that isn't a log line, like the telemetry
        // report. Never blocks, same as log lines.
        static void raw(std::string_view text);

    private:
        static constexpr Level HistoryLevel = Level::Info;

        void write(Level level, std::string_view fmt, std::format_args args) const;

        const char* m_tag;

        static std::atomic<Level> s_level;
        static std::atomic<bool> s_serialReady;
        static std::atomic<uint32_t> s_dropped;  // serial lines skipped since the last write
        static constinit LogHistory s_history;
    };
}
