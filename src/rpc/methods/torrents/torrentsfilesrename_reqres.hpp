#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsFilesRenameReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
        int                     file_index;
        std::string             file_path;
    };

    PORLA_JSON_REQUEST(
        TorrentsFilesRenameReq,
        info_hash,
        session_id,
        file_index,
        file_path)
}
