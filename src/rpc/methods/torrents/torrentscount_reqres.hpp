#pragma once

#include <map>
#include <optional>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsCountReq
    {
        int session_id;
    };

    struct TorrentsCountRes
    {
        std::map<std::string, int> categories;
        int                        downloading;
        int                        downloading_queued;
        int                        error;
        int                        finished;
        int                        paused;
        int                        seeding;
        int                        seeding_queued;
        int                        total;
        std::map<std::string, int> tags;
        std::map<std::string, int> trackers;
    };

    PORLA_JSON_REQUEST(
        TorrentsCountReq,
        session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsCountRes,
        categories,
        downloading,
        downloading_queued,
        error,
        finished,
        paused,
        seeding,
        seeding_queued,
        total,
        tags,
        trackers)
}
