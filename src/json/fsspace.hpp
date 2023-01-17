#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/fsspace_reqres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void from_json(const json& j, FsSpaceReq& req)
    {
        j["path"].get_to(req.path);
    }

    static void to_json(json& j, const FsSpaceRes& res)
    {
        j = {
            {"available", res.available},
            {"capacity", res.capacity},
            {"free", res.free}
        };
    }
}
