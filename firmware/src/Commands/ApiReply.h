#pragma once
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <utility>
#include "API/API.h"
#include "Commands/Command.h"

namespace Commands
{
    inline std::unexpected<Interface::Error> fail(ApiError error)
    {
        return Interface::fail(error.code, std::move(error.msg), std::move(error.arg));
    }

    inline std::unexpected<Interface::Error> missing(const char* key)
    {
        return Interface::fail(Interface::Code::MissingArg, std::string("missing ") + key, key);
    }

    inline std::unexpected<Interface::Error> wrongType(const char* key, const char* expected)
    {
        return Interface::fail(Interface::Code::InvalidArg, std::string(key) + " must be " + expected, key);
    }

    // Optional arguments: nullopt when absent, an error when present but of
    // the wrong type.
    template <typename T>
    using Opt = std::expected<std::optional<T>, Interface::Error>;

    inline Opt<int32_t> optInt(const JsonDocument& req, const char* key)
    {
        const JsonVariantConst v = req[key];
        if (v.isNull()) return std::nullopt;
        if (!v.is<int32_t>()) return std::unexpected(wrongType(key, "an integer").error());
        return v.as<int32_t>();
    }

    inline Opt<bool> optBool(const JsonDocument& req, const char* key)
    {
        const JsonVariantConst v = req[key];
        if (v.isNull()) return std::nullopt;
        if (!v.is<bool>()) return std::unexpected(wrongType(key, "true or false").error());
        return v.as<bool>();
    }

    inline Opt<std::string> optString(const JsonDocument& req, const char* key)
    {
        const JsonVariantConst v = req[key];
        if (v.isNull()) return std::nullopt;
        if (!v.is<const char*>()) return std::unexpected(wrongType(key, "a string").error());
        return std::string(v.as<const char*>());
    }

    // For commands that can't be undone: the request must say "confirm": true.
    inline std::expected<void, Interface::Error> requireConfirm(const JsonDocument& req)
    {
        if (req["confirm"] == true) return {};
        return std::unexpected(
            Interface::fail(Interface::Code::Forbidden, "confirm required", "confirm").error());
    }
}
