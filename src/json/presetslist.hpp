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
        j = {
            {"presets", json::object()}
        };

        for (auto const& [key,preset] : res.presets)
        {
            j["presets"][key] = json::object();
            j["presets"][key]["category"]        = preset.category        ? json(preset.category.value())        : json();
            j["presets"][key]["download_limit"]  = preset.download_limit  ? json(preset.download_limit.value())  : json();
            j["presets"][key]["max_connections"] = preset.max_connections ? json(preset.max_connections.value()) : json();
            j["presets"][key]["max_uploads"]     = preset.max_uploads     ? json(preset.max_uploads.value())     : json();
            j["presets"][key]["save_path"]       = preset.save_path       ? json(preset.save_path.value())       : json();
            j["presets"][key]["session"]         = preset.session         ? json(preset.session.value())         : json();
            j["presets"][key]["tags"]            = !preset.tags.empty()   ? json(preset.tags)                    : json();
            j["presets"][key]["upload_limit"]    = preset.upload_limit    ? json(preset.upload_limit.value())    : json();
        }
    }
}
