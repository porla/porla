#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsMigrateReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
        int                     target_session_id;
    };

    struct TorrentsMigrateRes
    {
        libtorrent::info_hash_t info_hash;
        int                     session_id;
    };

    PORLA_JSON_REQUEST(
        TorrentsMigrateReq,
        info_hash,
        session_id,
        target_session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsMigrateRes,
        info_hash,
        session_id)
}
