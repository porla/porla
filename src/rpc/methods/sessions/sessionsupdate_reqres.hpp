#pragma once

#include <map>
#include <optional>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsUpdateReq
    {
        int                                   id;
        std::string                           name;
        bool                                  is_default;
        std::map<std::string, nlohmann::json> metadata;
        std::optional<int>                    timer_dht_stats;
        std::optional<int>                    timer_save_state;
        std::optional<int>                    timer_session_stats;
        std::optional<int>                    timer_torrent_updates;
    };

    PORLA_JSON_REQUEST(
        SessionsUpdateReq,
        id,
        name,
        is_default,
        metadata,
        timer_dht_stats,
        timer_save_state,
        timer_session_stats,
        timer_torrent_updates)
}
