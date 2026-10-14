#pragma once

#include <map>
#include <optional>
#include <string>

#include <libtorrent/settings_pack.hpp>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsAddReq
    {
        std::string                                          name;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
        std::optional<std::string>                           settings_base;
        std::optional<std::map<std::string, nlohmann::json>> settings;
        std::optional<int>                                   timer_dht_stats;
        std::optional<int>                                   timer_save_state;
        std::optional<int>                                   timer_session_stats;
        std::optional<int>                                   timer_torrent_updates;
    };

    struct SessionsAddRes
    {
        int id;
    };

    PORLA_JSON_REQUEST(
        SessionsAddReq,
        name,
        metadata,
        settings_base,
        settings,
        timer_dht_stats,
        timer_save_state,
        timer_session_stats,
        timer_torrent_updates)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        SessionsAddRes,
        id)
}
