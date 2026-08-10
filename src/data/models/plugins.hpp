#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class Plugins
    {
    public:
        struct Plugin
        {
            int                                   id;
            std::string                           path;
            std::optional<std::string>            config;
            std::map<std::string, nlohmann::json> metadata;
        };

        static std::optional<Plugin> GetById(sqlite3* db, int id);
        static int Insert(sqlite3* db, const Plugin& plugin);
        static std::vector<Plugin> List(sqlite3* db);
        static void Remove(sqlite3* db, int id);
        static void Update(sqlite3* db, const Plugin& plugin);
    };
}
