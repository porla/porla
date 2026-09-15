#pragma once

#include <string>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class KeyValueStore
    {
    public:
        static nlohmann::json Get(sqlite3* db, const std::string& key);
        static void Set(sqlite3* db, const std::string& key, const nlohmann::json& value);
    };
}
