#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsPropertiesSetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int> download_limit;
        std::optional<std::map<std::string, bool>> flags;
        std::optional<int> max_connections;
        std::optional<int> max_uploads;
        std::optional<int> upload_limit;
    };

    struct TorrentsPropertiesSetRes
    {
    };
}
