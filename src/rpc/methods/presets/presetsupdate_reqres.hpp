#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include <libtorrent/torrent_flags.hpp>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Presets
{
    struct PresetsUpdateReq
    {
        int                                                  id;
        std::string                                          name;
        std::optional<bool>                                  is_default;
        std::optional<std::string>                           category;
        std::optional<int>                                   download_limit;
        std::optional<lt::torrent_flags_t>                   flags;
        std::optional<lt::torrent_flags_t>                   flags_mask;
        std::optional<int>                                   max_connections;
        std::optional<int>                                   max_uploads;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
        std::optional<int>                                   session_id;
        std::optional<std::string>                           save_path;
        std::optional<std::string>                           storage_mode;
        std::unordered_set<std::string>                      tags;
        std::optional<int>                                   upload_limit;
    };

    PORLA_JSON_REQUEST(
        PresetsUpdateReq,
        id,
        name,
        is_default,
        category,
        download_limit,
        flags,
        flags_mask,
        max_connections,
        max_uploads,
        metadata,
        session_id,
        save_path,
        storage_mode,
        tags,
        upload_limit)
}
