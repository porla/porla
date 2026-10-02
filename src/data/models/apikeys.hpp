#pragma once

#include <string>
#include <vector>

#include <sqlite3.h>

namespace porla::Data::Models
{
    struct ApiKeys
    {
        struct ApiKey
        {
            std::string                 id;
            std::string                 name;
            std::vector<char>           secret_hash;
            std::int64_t                created_at;
            std::optional<std::int64_t> expires_at;
        };

        static std::optional<ApiKey> GetById(sqlite3* db, std::string_view id);

        static void Insert(
            sqlite3* db,
            std::string_view id,
            std::string_view name,
            std::vector<char> secret_hash,
            std::optional<std::int64_t> expires_at);

        static std::vector<ApiKey> List(sqlite3* db);

        static void Remove(sqlite3* db, std::string_view id);
    };
}
