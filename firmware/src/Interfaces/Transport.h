#pragma once
#include <string>

namespace Interface
{
    struct Transport 
    {
        virtual void send(const std::string& data) = 0;
        virtual std::string readLine() = 0;
        virtual bool isAvailable() = 0;
        virtual std::string remoteAddress() = 0;
        virtual void close() = 0;
    };

}