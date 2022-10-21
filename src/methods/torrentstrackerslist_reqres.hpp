#pragma once

#include <string>
#include <vector>

#include <libtorrent/announce_entry.hpp>
#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsTrackersListReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsTrackersListRes
    {
        std::vector<libtorrent::announce_entry> trackers;
    };
}
