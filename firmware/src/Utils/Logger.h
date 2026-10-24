#pragma once
#include <Arduino.h>
#include <cstdarg>

namespace Utils
{
    class Logger
    {
    public:
        enum class Level
        {
            DEBUG,
            INFO
        };
    private:
        static bool m_enabled;
        static Level m_level;

    public:
        
        static void enable(uint32_t baudrate = 9600)
        {
            if(!m_enabled)
            {
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
                // USB serial (HWCDC). When the cable is in but nothing on
                // the host reads, each write retries 20x the TX timeout
                // (100 ms default) before giving up, stalling the calling
                // task ~2 s. Keep the timeout short, and the buffer big
                // enough that a whole report or reply fits without waiting
                // when a monitor is reading. Both must be set before begin().
                Serial.setTxBufferSize(4096);
                Serial.setTxTimeoutMs(2);
#endif
                Serial.begin(baudrate);
                m_enabled = true;
            }
        }

        static void setLevel(Level level)
        {
            m_level = level;
        }
        
        static void print(const char* str)
        {
            if(m_enabled)
            {
                Serial.print(str);
            }
        }

        static void println(const char* str)
        {
            if(m_enabled)
            {
                Serial.println(str);
            }
        }

        // format(printf) makes the compiler check the arguments against the
        // format string, like it does for ::printf.
        [[gnu::format(printf, 1, 2)]]
        static void printf(const char* format, ...)
        {
            if(m_enabled)
            {
                va_list args;
                va_start(args, format);
                char buf[256];
                int n = vsnprintf(buf, sizeof(buf), format, args);
                va_end(args);
                if (n > 0) Serial.write(buf, std::min<size_t>(n, sizeof(buf) - 1));
            }
        }
    };
}