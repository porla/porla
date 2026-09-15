#pragma once

#include <optional>
#include <vector>

#include <libtorrent/file_storage.hpp>
#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsFilesListReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsFilesListRes
    {
        libtorrent::file_storage  file_storage;
        libtorrent::renamed_files renamed_files;
    };
}
