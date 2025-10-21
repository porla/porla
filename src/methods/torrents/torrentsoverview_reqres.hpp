#pragma once

#include <map>
#include <optional>
#include <stdint.h>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace porla::Methods::Torrents
{
    struct TorrentsOverviewReq
    {
        std::optional<std::map<std::string, nlohmann::json>> filters;
    };

    struct TorrentsOverviewSession
    {
        int                                     session_id;
        std::string                             session_name;
        std::map<std::string, std::uint64_t>    torrents_per_category;
        std::uint64_t                           torrents_errors;
        std::vector<std::vector<std::uint64_t>> torrents_per_flags;
        std::map<std::string, std::uint64_t>    torrents_per_state;
        std::map<std::string, std::uint64_t>    torrents_per_tag;
        std::map<std::string, std::uint64_t>    torrents_per_tracker;
        std::uint64_t                           torrents_total;
    };

    struct TorrentsOverviewRes
    {
        std::vector<TorrentsOverviewSession> sessions;
    };
}
