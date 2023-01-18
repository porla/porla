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

    static void to_json(json& j, const FsSpaceQuota& res)
    {
        j = {
            {"blocks_limit_hard", res.blocks_limit_hard},
            {"blocks_limit_soft", res.blocks_limit_soft},
            {"blocks_time", res.blocks_time},
            {"current_inodes", res.current_inodes},
            {"current_space", res.current_space},
            {"inodes_limit_hard", res.inodes_limit_hard},
            {"inodes_limit_soft", res.inodes_limit_soft},
            {"inodes_time", res.inodes_time},
        };
    }

    static void to_json(json& j, const FsSpaceRes& res)
    {
        j = {
            {"available", res.available},
            {"capacity", res.capacity},
            {"free", res.free},
            {"quota", res.quota.has_value() ? res.quota.value() : json()}
        };
    }
}
