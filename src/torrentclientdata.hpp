#pragma once

#include <map>
#include <optional>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "mediainfo/container.hpp"

namespace porla
{
    struct TorrentClientData
    {
        std::optional<std::string>                            category;
        std::optional<MediaInfo::Container>                   mediainfo;
        std::optional<bool>                                   mediainfo_enabled;
        std::optional<std::map<int, std::unordered_set<int>>> mediainfo_file_pieces_completed;
        std::optional<std::map<int, std::unordered_set<int>>> mediainfo_file_pieces_wanted;
        std::optional<std::map<std::string, nlohmann::json>>  metadata;
        std::optional<std::unordered_set<std::string>>        tags;
    };
}
