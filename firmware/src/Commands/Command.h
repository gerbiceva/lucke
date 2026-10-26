#pragma once
#include <ArduinoJson.h>
#include "Interfaces/Transport.h"

namespace Interface
{
    struct CommandHandler
    {
        virtual ~CommandHandler() = default;
        virtual void handle(JsonDocument& req, Transport& transport) = 0;
    };
}
