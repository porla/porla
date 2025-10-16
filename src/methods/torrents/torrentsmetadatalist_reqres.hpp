#pragma once

#include <map>
#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>
#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct TorrentsMetadataListReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsMetadataListRes
    {
        std::map<std::string, nlohmann::json> metadata;
    };
}
