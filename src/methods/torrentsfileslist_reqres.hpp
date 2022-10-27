#pragma once

#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsFilesListReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsFilesListRes
    {
        struct FileItem
        {
            int index;
            std::string name;
        };

        std::vector<FileItem> files;
    };
}
