#pragma once

#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

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
}
