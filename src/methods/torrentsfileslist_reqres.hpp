#pragma once

#include <vector>

#include <libtorrent/file_storage.hpp>
#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsFilesListReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsFilesListRes
    {
        libtorrent::file_storage file_storage;
    };
}
