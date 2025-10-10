#pragma once

#include <map>
#include <string>

#include <libtorrent/info_hash.hpp>
#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct TorrentsMetadataListReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsMetadataListRes
    {
        std::map<std::string, nlohmann::json> metadata;
    };
}
