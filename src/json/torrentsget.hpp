#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "../methods/torrentsget_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const json& j, porla::Methods::TorrentsGetReq& req)
    {
    }

    static void to_json(json& j, const porla::Methods::TorrentsGetRes& res)
    {
        j = json{
            {"foo", "bar"}
        };
    }
}
