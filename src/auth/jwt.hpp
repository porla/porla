#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace porla::Auth
{
    // HS256 JSON Web Tokens for user logins.
    struct Jwt
    {
        // A token for `subject`, issued by porla at `now` and valid for `ttl`.
        static std::string Sign(
            const std::string&                    secret_key,
            const std::string&                    subject,
            std::chrono::seconds                  ttl,
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now());

        // The token's subject, if it is an HS256 token issued by porla, signed with `secret_key`
        // and valid at `now`. Never throws.
        static std::optional<std::string> Verify(
            const std::string&                    secret_key,
            const std::string&                    token,
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now());
    };
}
