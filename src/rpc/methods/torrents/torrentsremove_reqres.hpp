#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsRemoveReq
    {
        libtorrent::info_hash_t              info_hash;
        std::optional<int>                   session_id;
        std::optional<bool>                  remove_data;
    };

    PORLA_JSON_REQUEST(
        TorrentsRemoveReq,
        info_hash,
        session_id,
        remove_data)
}
