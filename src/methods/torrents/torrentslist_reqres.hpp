#pragma once

#include <map>
#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

namespace porla::Methods
{
    struct TorrentsListFilters
    {
        std::optional<std::string>                     category;
        std::optional<bool>                            errc;
        std::optional<std::unordered_set<std::string>> flags;
        std::optional<std::string>                     query;
        std::optional<int>                             session_id;
        std::optional<std::string>                     save_path;
        std::optional<std::unordered_set<std::string>> state;
        std::optional<std::unordered_set<std::string>> status;
        std::optional<std::unordered_set<std::string>> tags;
    };

    struct TorrentsListReq
    {
        std::optional<TorrentsListFilters> filters;
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
