#pragma once

#include <map>
#include <optional>
#include <string>

#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>

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
}
