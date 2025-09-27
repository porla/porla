#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionslist_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json& j, SessionsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes::Item,
        id,
        name,
        is_dht_running,
        is_listening,
        is_paused,
        torrents_total)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsListRes,
        sessions)
}
