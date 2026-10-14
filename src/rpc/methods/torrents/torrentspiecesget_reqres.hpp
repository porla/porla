#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPiecesGetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsPiecesGetRes
    {
        lt::typed_bitfield<lt::piece_index_t> pieces;
        lt::typed_bitfield<lt::piece_index_t> verified_pieces;
    };

    PORLA_JSON_REQUEST(
        TorrentsPiecesGetReq,
        info_hash,
        session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsPiecesGetRes,
        pieces,
        verified_pieces)
}
