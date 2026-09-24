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

        bool IsAuthenticated() const { return kind != Kind::Anon; }
    };
}
