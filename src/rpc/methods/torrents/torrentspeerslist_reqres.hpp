#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/peer_info.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPeersListReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsPeersListRes
    {
        std::vector<libtorrent::peer_info> peers;
    };

    PORLA_JSON_REQUEST(
        TorrentsPeersListReq,
        info_hash,
        session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsPeersListRes,
        peers)
}
