#pragma once

#include <map>
#include <string>

#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>

namespace porla::Methods::Sessions
{
    struct SessionsGetReq
    {
        int id;
    };

    struct SessionsGetRes
    {
        struct SessionState
        {

            bool  is_listening;
            bool  is_paused;
            int   torrents_total;
        };

        struct Session
        {
            int                                   id;
            std::string                           name;
            bool                                  is_default;
            std::map<std::string, nlohmann::json> metadata;
            std::optional<SessionState>           state;
        };

        Session session;
    };
}
