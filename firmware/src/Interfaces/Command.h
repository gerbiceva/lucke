#pragma once
#include <ArduinoJson.h>
#include "Transport.h"

namespace Interface
{
    struct CommandHandler
    {
        virtual void handle(JsonDocument& req, Transport& transport) = 0;
    };
}
