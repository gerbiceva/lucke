#pragma once
#include <Arduino.h>
#include "StringUtils.h"

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

        template<typename... Args>
        static void printf(const char* format, Args&&... args)
        {
            if(m_enabled)
            {
                Serial.printf(format, args...);
            }
        }
    };
}