#pragma once
#include <expected>
#include <string>
#include <string_view>
#include <ArduinoJson.h>

namespace Interface
{
    // Who sent the command, passed to the API as the change origin.
    struct Context
    {
        std::string from;
    };

    // The document to send back, or an error the dispatcher sends as
    // {"err": ...}. The error must point to static storage (a literal or
    // toString()).
    using Reply = std::expected<JsonDocument, std::string_view>;

    struct CommandHandler
    {
        virtual ~CommandHandler() = default;
        virtual Reply handle(const JsonDocument& req, const Context& ctx) = 0;
    };
}
