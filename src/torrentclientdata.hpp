#pragma once

#include <map>
#include <optional>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace porla
{
    struct TorrentClientData
    {
        std::optional<std::string>            category;
        std::optional<nlohmann::json>         mediainfo;
        std::unordered_set<int>               mediainfo_pieces_completed;
        std::unordered_set<int>               mediainfo_pieces_wanted;
        std::map<std::string, nlohmann::json> metadata;
        std::unordered_set<std::string>       tags;
    };
}
