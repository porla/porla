#pragma once

#include <map>
#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsOverviewReq
    {
    };

    struct TorrentsOverviewRes
    {
        std::map<std::string, std::uint64_t> torrents_per_category;
        std::map<std::string, std::uint64_t> torrents_per_session;
        std::map<std::string, std::uint64_t> torrents_per_state;
        std::map<std::string, std::uint64_t> torrents_per_tag;
        std::map<std::string, std::uint64_t> torrents_per_tracker;
    };
}
