#pragma once
#include <Arduino.h>
#include "Interfaces/Transport.h"

namespace Net
{
    class SerialTransport : public Interface::Transport
    {
    public:
        void send(const std::string& data) override;
        std::string readLine() override;
        bool isAvailable() override;
        std::string remoteAddress() override;
        void close() override;
    };
}
