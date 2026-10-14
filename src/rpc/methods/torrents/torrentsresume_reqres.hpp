#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsResumeReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    PORLA_JSON_REQUEST(
        TorrentsResumeReq,
        info_hash,
        session_id)
}
