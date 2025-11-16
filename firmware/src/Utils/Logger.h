#pragma once
#include <Arduino.h>

namespace Utils
{
    class Logger
    {
        static bool m_enabled;
    public:
        static void enable()
        {
            if(!m_enabled)
            {
                Serial.begin(9600);
                m_enabled = true;
            	sleep(1);
            }
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