#pragma once

#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsRecheckReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsRecheckRes
    {
    };
}
