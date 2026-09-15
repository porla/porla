#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

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
}
