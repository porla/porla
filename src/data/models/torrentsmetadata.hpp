#pragma once

#include <map>
#include <string>

#include <libtorrent/info_hash.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class TorrentsMetadata
    {
    public:
        static std::map<std::string, nlohmann::json> GetAll(sqlite3* db, const libtorrent::info_hash_t& hash);
        static void RemoveAll(sqlite3* db, const libtorrent::info_hash_t& hash);
        static void Set(sqlite3* db, const libtorrent::info_hash_t& hash, const std::string& key, const nlohmann::json& value);
    };
}
