#pragma once

#include <optional>
#include <vector>

#include <libtorrent/download_priority.hpp>
#include <libtorrent/info_hash.hpp>
#include <libtorrent/units.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsFilesPrioritizeReq
    {
        struct FilePrio
        {
            libtorrent::file_index_t        index;
            libtorrent::download_priority_t priority;
        };

        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
        std::vector<FilePrio>   priorities;
    };

    struct TorrentsFilesPrioritizeRes
    {
    };
}
