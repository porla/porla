#pragma once

#include <optional>
#include <vector>

#include <libtorrent/file_storage.hpp>
#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsFilesListReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsFilesListRes
    {
        libtorrent::file_storage file_storage;
    };
}
