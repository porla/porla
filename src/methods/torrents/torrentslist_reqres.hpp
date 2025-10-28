#pragma once

#include <map>
#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

namespace porla::Methods
{
    struct TorrentsListReq
    {
        std::optional<std::map<std::string, nlohmann::json>> filters;
        std::optional<int> page;
        std::optional<int> page_size;
        std::optional<std::string> order_by;
        std::optional<std::string> order_by_dir;
    };

    struct TorrentsListRes
    {
        std::string                             order_by;
        std::string                             order_by_dir;
        int                                     page;
        int                                     page_size;
        std::vector<libtorrent::torrent_status> torrents;
        int                                     torrents_total;
        int                                     torrents_total_unfiltered;
    };
}
