#pragma once

#include <string>
#include <unordered_set>

#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    struct SessionSettings
    {
        static void Apply(sqlite3* db, libtorrent::settings_pack& settings);
        static void Update(sqlite3* db, const std::string& name, const nlohmann::json& value);

        static const std::unordered_set<std::string> BlockedKeys;
    };
}
