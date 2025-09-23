#pragma once

#include <map>
#include <string>

#include <stdint.h>

namespace porla::Methods::Torrents
{
    struct TorrentsOverviewReq
    {
    };

    struct TorrentsOverviewSession
    {
        std::map<std::string, std::uint64_t> torrents_per_category;
        std::map<std::string, std::uint64_t> torrents_per_state;
        std::map<std::string, std::uint64_t> torrents_per_tag;
        std::map<std::string, std::uint64_t> torrents_per_tracker;
        std::uint64_t                        torrents_total;
    };

    struct TorrentsOverviewRes
    {
        std::map<std::string, TorrentsOverviewSession> sessions;
    };
}
