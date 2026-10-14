#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsFilesProgressReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsFilesProgressRes
    {
        std::vector<std::int64_t> progress;
    };

    PORLA_JSON_REQUEST(TorrentsFilesProgressReq, info_hash, session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(TorrentsFilesProgressRes, progress)
}
