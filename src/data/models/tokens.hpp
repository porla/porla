#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <sqlite3.h>

namespace porla::Data::Models
{
    // User sessions and API keys: random secrets stored as hashes, looked up by id.
    struct Tokens
    {
        struct Token
        {
            std::string                 id;
            std::string                 kind;           // "session" or "api_key"
            std::optional<int>          user_id;
            std::optional<std::string>  username;       // joined from users, for sessions
            std::optional<std::string>  name;
            std::vector<char>           secret_hash;
            std::int64_t                created_at;
            std::optional<std::int64_t> expires_at;
            std::optional<std::int64_t> last_used_at;
            std::optional<std::string>  user_agent;
            std::optional<std::string>  remote_address;
        };

        static std::optional<Token> GetById(sqlite3* db, std::string_view id);
        static void Insert(sqlite3* db, const Token& token);
        static std::vector<Token> List(sqlite3* db, std::string_view kind);
        static void Remove(sqlite3* db, std::string_view id);
        static int RemoveExpiredSessions(sqlite3* db, std::int64_t now);
        static void Touch(sqlite3* db, std::string_view id, std::int64_t now, std::optional<std::int64_t> expires_at);
    };
}
