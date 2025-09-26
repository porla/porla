#pragma once

#include <optional>
#include <string>

namespace porla::Methods
{
    struct TorrentsAddReq
    {
        std::optional<std::string>                           category;
        std::optional<int>                                   download_limit;
        std::optional<std::vector<std::string>>              http_seeds;
        std::optional<std::string>                           magnet_uri;
        std::optional<int>                                   max_connections;
        std::optional<int>                                   max_uploads;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
        std::optional<std::string>                           name;
        std::optional<int>                                   preset_id;
        std::optional<std::string>                           save_path;
        std::optional<int>                                   session_id;
        std::optional<std::unordered_set<std::string>>       tags;
        std::optional<std::string>                           ti;
        std::optional<std::vector<std::string>>              trackers;
        std::optional<int>                                   upload_limit;
        std::optional<std::vector<std::string>>              url_seeds;
    };
}
