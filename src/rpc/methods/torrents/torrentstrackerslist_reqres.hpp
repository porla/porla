#pragma once

#include <optional>
#include <string>
#include <vector>

#include <libtorrent/announce_entry.hpp>
#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsTrackersListReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsTrackersListRes
    {
        std::vector<libtorrent::announce_entry> trackers;
    };
}
