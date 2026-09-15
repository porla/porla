#pragma once

#include <optional>
#include <vector>

#include <libtorrent/download_priority.hpp>
#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsFilesPrioritiesReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsFilesPrioritiesRes
    {
        std::vector<libtorrent::download_priority_t> priorities;
    };
}
