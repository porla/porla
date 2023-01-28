#pragma once

#include <libtorrent/info_hash.hpp>

#include "../mediainfo/container.hpp"

namespace porla::Methods
{
    struct TorrentsMediaInfoReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsMediaInfoRes
    {
        std::optional<MediaInfo::Container> mediainfo;
    };
}
