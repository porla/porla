#pragma once

#include <map>
#include <optional>
#include <string>

namespace porla::Methods::Torrents
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
}
