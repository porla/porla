#pragma once

#include <string>
#include <unordered_set>

#include <libtorrent/torrent_flags.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Presets
{
    struct PresetsGetReq
    {
        int id;
    };

    struct PresetsGetRes
    {
        struct Preset
        {
            int                                                    id;
            std::string                                            name;
            bool                                                   is_default;
            std::map<std::string, nlohmann::json>                  metadata;

            std::optional<std::string>                             category;
            std::optional<int>                                     download_limit;
            std::optional<lt::torrent_flags_t>                     flags;
            std::optional<lt::torrent_flags_t>                     flags_mask;
            std::optional<int>                                     max_connections;
            std::optional<int>                                     max_uploads;
            std::optional<std::string>                             save_path;
            std::optional<int>                                     session_id;
            std::optional<std::string>                             storage_mode;
            std::unordered_set<std::string>                        tags;
            std::optional<int>                                     upload_limit;
        };

        Preset preset;
    };

    PORLA_JSON_REQUEST(PresetsGetReq, id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        PresetsGetRes::Preset,
        id,
        name,
        is_default,
        metadata,
        category,
        download_limit,
        flags,
        flags_mask,
        max_connections,
        max_uploads,
        save_path,
        session_id,
        storage_mode,
        tags,
        upload_limit)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        PresetsGetRes,
        preset)
}
