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
        struct Session
        {
            int                                   id;
            std::string                           name;
            bool                                  is_default;
            bool                                  is_listening;
            bool                                  is_paused;
            std::map<std::string, nlohmann::json> metadata;
            int                                   torrents_total;
        };

        Session session;
    };
}
