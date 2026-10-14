#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPropertiesSetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;

        std::optional<std::string>                     category;
        std::optional<int>                             download_limit;
        std::optional<libtorrent::torrent_flags_t>     flags;
        std::optional<libtorrent::torrent_flags_t>     flags_mask;
        std::optional<int>                             max_connections;
        std::optional<int>                             max_uploads;
        std::optional<std::unordered_set<std::string>> tags;
        std::optional<int>                             upload_limit;
    };

    PORLA_JSON_REQUEST(
        TorrentsPropertiesSetReq,
        info_hash,
        session_id,
        category,
        download_limit,
        flags,
        flags_mask,
        max_connections,
        max_uploads,
        tags,
        upload_limit)
}
