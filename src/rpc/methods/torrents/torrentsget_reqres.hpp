#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsGetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsGetRes
    {
        libtorrent::torrent_status torrent;
    };

    PORLA_JSON_REQUEST(TorrentsGetReq, info_hash, session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(TorrentsGetRes, torrent)
}
