#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrents/torrentsremovereq.hpp"
#include "../methods/torrents/torrentsremoveres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsRemoveReq,
        info_hashes,
        remove_data)

    static void to_json(json& j, const porla::Methods::TorrentsRemoveRes& res)
    {
    }
}
