#pragma once

#include <map>
#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
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

    PORLA_JSON_REQUEST(
        TorrentsListFilters,
        category,
        errc,
        flags,
        query,
        session_id,
        save_path,
        state,
        status,
        tags)

    PORLA_JSON_REQUEST(
        TorrentsListReq,
        filters,
        page,
        page_size,
        order_by,
        order_by_dir)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsListRes,
        order_by,
        order_by_dir,
        page,
        page_size,
        torrents,
        torrents_total,
        torrents_total_unfiltered)
}
