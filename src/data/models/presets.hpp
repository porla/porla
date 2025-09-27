#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class Presets
    {
    public:
        struct Preset
        {
            int                             id;
            std::string                     name;
            std::optional<std::string>      category;
            std::optional<int>              download_limit;
            std::optional<int>              max_connections;
            std::optional<int>              max_uploads;
            std::optional<nlohmann::json>   metadata;
            std::optional<int>              session_id;
            std::optional<std::string>      save_path;
            std::optional<std::string>      storage_mode;
            std::unordered_set<std::string> tags;
            std::optional<int>              upload_limit;
        };

        static void ForEach(sqlite3* db, const std::function<void(const Preset&)>& cb);
        static std::optional<Preset> GetById(sqlite3* db, int id);
        static std::optional<Preset> GetByName(sqlite3* db, const std::string& name);
        static int Insert(sqlite3* db, const std::string& name);
        static void Update(sqlite3* db, const Preset& preset);
    };
}
