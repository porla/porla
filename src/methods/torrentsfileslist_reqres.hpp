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
        std::vector<std::int64_t> file_progress;
        libtorrent::file_storage  file_storage;
    };
}
