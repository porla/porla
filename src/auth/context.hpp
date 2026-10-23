#pragma once

#include <optional>
#include <string>
#include <unordered_set>

namespace porla::Auth
{
    struct Context
    {
        enum class Kind { Anon, User, Machine };

        Kind                            kind    = Kind::Anon;
        std::optional<std::string>      subject = std::nullopt;

        // The session or API key that authenticated the request.
        std::optional<std::string>      token_id = std::nullopt;

        // Where the request came from (set by the HTTP handler).
        std::string                     user_agent;
        std::string                     remote_address;

        bool IsAuthenticated() const { return kind != Kind::Anon; }
    };
}
