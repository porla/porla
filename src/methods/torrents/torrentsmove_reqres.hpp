#pragma once

#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsMoveReq
    {
        std::optional<std::string> flags;
        libtorrent::info_hash_t info_hash;
        std::string path;
    };

    struct TorrentsMoveRes
    {
    };
}
