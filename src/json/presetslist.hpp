#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/presetslist_reqres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void from_json(const json& j, PresetsListReq& req)
    {
    }

    static void to_json(json& j, const PresetsListRes& res)
    {
        j = json::object();
        for (auto const& [key,preset] : res.presets)
        {
            j[key] = json::object();
            j[key]["download_limit"]  = preset.download_limit  ? json(preset.download_limit.value())  : json();
            j[key]["max_connections"] = preset.max_connections ? json(preset.max_connections.value()) : json();
            j[key]["max_uploads"]     = preset.max_uploads     ? json(preset.max_uploads.value())     : json();
            j[key]["save_path"]       = preset.save_path       ? json(preset.save_path.value())       : json();
            j[key]["upload_limit"]    = preset.upload_limit    ? json(preset.upload_limit.value())    : json();
        }
    }
}
