#pragma once

#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsMoveReq
    {
        std::optional<std::string> flags;
        libtorrent::info_hash_t    info_hash;
        std::string                path;
        std::optional<int>         session_id;
    };

    PORLA_JSON_REQUEST(
        TorrentsMoveReq,
        flags,
        info_hash,
        path,
        session_id)
}
