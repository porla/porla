#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

#include <sqlite3.h>

#include "../data/models/tokens.hpp"

namespace porla::Auth::Tokens
{
    // a user keeps at most this many sessions
    constexpr int kMaxSessionsPerUser = 50;

    // a user session expires after this long without use.
    constexpr std::chrono::seconds kSessionIdleTimeout = std::chrono::days{30};

    // last_used_at (and a session's expiry) is written at most this often per token.
    constexpr std::chrono::seconds kTouchInterval = std::chrono::minutes{5};

    struct Issued
    {
        std::string id;
        std::string token; // porla_<id>_<secret> - shown once, only its hash is stored
    };

    // generates an id and secret for the row (id, secret_hash and created_at are filled in) and
    // stores it.
    Issued Issue(sqlite3* db, Data::Models::Tokens::Token row);

    // Parses porla_<id>_<secret>, looks the row up and checks expiry and the secret. touches the
    // row (and slides a session's expiry) at most every kTouchInterval.
    std::optional<Data::Models::Tokens::Token> Verify(sqlite3* db, std::string_view token);
}
