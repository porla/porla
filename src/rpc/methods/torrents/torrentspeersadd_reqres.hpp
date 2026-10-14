#pragma once

#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPeersAddReq
    {
        libtorrent::info_hash_t                        info_hash;
        std::optional<int>                             session_id;
        std::vector<std::tuple<std::string, uint16_t>> peers;
    };

    PORLA_JSON_REQUEST(
        TorrentsPeersAddReq,
        info_hash,
        session_id,
        peers)
}
