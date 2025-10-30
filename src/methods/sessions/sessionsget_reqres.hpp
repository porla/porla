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
            std::map<std::string, nlohmann::json> metadata;
            libtorrent::settings_pack             settings;
        };

        Session session;
    };
}
