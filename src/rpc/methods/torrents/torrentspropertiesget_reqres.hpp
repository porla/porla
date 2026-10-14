#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPropertiesGetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsPropertiesGetRes
    {
        int download_limit;
        libtorrent::torrent_flags_t flags;
        int max_connections;
        int max_uploads;
        int upload_limit;
    };

    PORLA_JSON_REQUEST(
        TorrentsPropertiesGetReq,
        info_hash,
        session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsPropertiesGetRes,
        download_limit,
        flags,
        max_connections,
        max_uploads,
        upload_limit)
}
