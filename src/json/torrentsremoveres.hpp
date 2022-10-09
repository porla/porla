#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsremoveres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void to_json(json& j, const porla::Methods::TorrentsRemoveRes& res)
    {
    }
}
