#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_flags.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsAddReq
    {
        std::optional<std::string>                           category;
        std::optional<int>                                   download_limit;
        std::optional<lt::torrent_flags_t>                   flags;
        std::optional<std::string>                           magnet_uri;
        std::optional<int>                                   max_connections;
        std::optional<int>                                   max_uploads;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
        std::optional<std::string>                           name;
        std::optional<std::string>                           preset;
        std::optional<int>                                   preset_id;
        std::optional<std::string>                           save_path;
        std::optional<int>                                   session_id;
        std::optional<std::unordered_set<std::string>>       tags;
        std::optional<std::string>                           ti;
        std::optional<std::vector<std::string>>              trackers;
        std::optional<int>                                   upload_limit;
        std::optional<std::vector<std::string>>              url_seeds;
    };

    struct TorrentsAddRes
    {
        libtorrent::info_hash_t info_hash;
        int                     session_id;
    };

    PORLA_JSON_REQUEST(
        TorrentsAddReq,
        category,
        download_limit,
        flags,
        magnet_uri,
        max_connections,
        max_uploads,
        metadata,
        name,
        preset,
        preset_id,
        save_path,
        session_id,
        tags,
        ti,
        trackers,
        upload_limit,
        url_seeds)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        TorrentsAddRes,
        info_hash,
        session_id)
}
