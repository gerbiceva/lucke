#pragma once
#include <string>
#include <ArduinoJson.h>

namespace Interface
{
    struct Transport
    {
        virtual ~Transport() = default;
        virtual void send(const std::string& data) = 0;
        virtual std::string readLine() = 0;
        virtual bool isAvailable() = 0;
        virtual std::string remoteAddress() = 0;
        virtual void close() = 0;

        void send(const JsonDocument& doc)
        {
            std::string data;
            serializeJson(doc, data);
            send(data);
        }
    };

}