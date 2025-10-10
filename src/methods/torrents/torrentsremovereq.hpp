#pragma once

#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsRemoveReq
    {
        std::vector<libtorrent::info_hash_t> info_hashes;
        bool remove_data;
    };
}
