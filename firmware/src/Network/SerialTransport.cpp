#include "SerialTransport.h"

namespace Net
{
    void SerialTransport::send(const std::string& data)
    {
        Serial.println(data.c_str());
    }

    std::string SerialTransport::readLine()
    {
        if (Serial.available())
        {
            return std::string(Serial.readStringUntil('\n').c_str());
        }
        return "";
    }

    bool SerialTransport::isAvailable()
    {
        return Serial.available();
    }

    std::string SerialTransport::remoteAddress()
    {
        return "serial";
    }

    void SerialTransport::close()
    {
        // Serial is always open
    }
}
