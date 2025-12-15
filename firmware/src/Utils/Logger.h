#pragma once
#include <Arduino.h>

namespace Utils
{
    class Logger
    {
    public:
        enum Level
        {
            DEBUG,
            INFO
        };
    private:
        static bool m_enabled;
        static Level m_level;

    public:
        
        static void enable(Level level = INFO)
        {
            m_level = level;
            if(!m_enabled)
            {
                Serial.begin(9600);
                m_enabled = true;
            }
        }
        
        static void print(const char* str)
        {
            if(m_enabled)
            {
                Serial.print(str);
            }
        }

        static void dprint(const char* str)
        {
            if(m_level == DEBUG)
            {
                print(str);
            }
        }

        static void println(const char* str)
        {
            if(m_enabled)
            {
                Serial.println(str);
            }
        }

        static void dprintln(const char* str)
        {
            if(m_level == DEBUG)
            {
                println(str);
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

        template<typename... Args>
        static void dprintf(const char* format, Args&&... args)
        {
            if(m_level == DEBUG)
            {
                printf(format, args...);
            }
        }

    };
}