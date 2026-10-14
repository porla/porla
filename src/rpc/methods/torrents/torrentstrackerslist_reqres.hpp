#pragma once

#include <optional>
#include <string>
#include <vector>

#include <libtorrent/announce_entry.hpp>
#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsTrackersListReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsTrackersListRes
    {
        std::vector<libtorrent::announce_entry> trackers;
    };

    PORLA_JSON_REQUEST(
        TorrentsTrackersListReq,
        info_hash,
        session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsTrackersListRes,
        trackers)
}
