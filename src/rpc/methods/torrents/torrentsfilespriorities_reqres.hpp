#pragma once

#include <optional>
#include <vector>

#include <libtorrent/download_priority.hpp>
#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsFilesPrioritiesReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsFilesPrioritiesRes
    {
        std::vector<libtorrent::download_priority_t> priorities;
    };

    PORLA_JSON_REQUEST(TorrentsFilesPrioritiesReq, info_hash, session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(TorrentsFilesPrioritiesRes, priorities)
}
