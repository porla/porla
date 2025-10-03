#pragma once

#include <string>
#include <set>

#include <nlohmann/json.hpp>

namespace porla::Methods::Presets
{
    struct PresetsGetReq
    {
        int id;
    };

    struct PresetsGetRes
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
}
