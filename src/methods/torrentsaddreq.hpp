#pragma once

#include <optional>
#include <string>

namespace porla::Methods
{
    struct TorrentsAddReq
    {
        std::optional<int> download_limit;
        std::optional<std::vector<std::string>> http_seeds;
        std::optional<std::string> magnet_uri;
        std::optional<int> max_connections;
        std::optional<int> max_uploads;
        std::optional<std::string> name;
        std::string save_path;
        std::optional<std::string> ti;
        std::optional<std::vector<std::string>> trackers;
        std::optional<int> upload_limit;
        std::optional<std::vector<std::string>> url_seeds;
    };
}
