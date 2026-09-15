#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsRemoveReq
    {
        libtorrent::info_hash_t              info_hash;
        std::optional<int>                   session_id;
        std::optional<bool>                  remove_data;
    };
}
