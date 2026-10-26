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

    // What a handler returns on success: the reply's "data", and whether the
    // device restarts because of this command (sent as "rebooting": true).
    struct Success
    {
        JsonDocument data;
        bool rebooting = false;
    };

    // The dispatcher sends this as {"ok": false, "err": {code, arg, msg}}.
    // `code` must be one of Code:: (static storage). `arg` may be empty.
    struct Error
    {
        std::string_view code;
        std::string msg;
        std::string arg = {};
    };

    using Reply = std::expected<Success, Error>;

    // Error codes, as documented in docs/apiv2.md.
    namespace Code
    {
        inline constexpr std::string_view InvalidJson = "invalid_json";
        inline constexpr std::string_view MissingCmd = "missing_cmd";
        inline constexpr std::string_view UnknownCmd = "unknown_cmd";
        inline constexpr std::string_view MissingArg = "missing_arg";
        inline constexpr std::string_view InvalidArg = "invalid_arg";
        inline constexpr std::string_view OutOfRange = "out_of_range";
        inline constexpr std::string_view NotFound = "not_found";
        inline constexpr std::string_view Forbidden = "forbidden";
    }

    inline std::unexpected<Error> fail(std::string_view code, std::string msg, std::string arg = {})
    {
        return std::unexpected(Error{code, std::move(msg), std::move(arg)});
    }

    struct CommandHandler
    {
        virtual ~CommandHandler() = default;
        virtual Reply handle(const JsonDocument& req, const Interface::Context& ctx) = 0;
    };
}
