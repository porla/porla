#pragma once

#include <string>

#include <sqlite3.h>

namespace porla::Data::Models
{
    struct ApiKeys
    {
        struct ApiKey
        {
            std::string id;
        };

        static void Insert(
            sqlite3* db,
            std::string_view id,
            std::string_view name,
            std::string_view secret_hash,
            std::optional<std::int64_t> expires_at);
    };
}
