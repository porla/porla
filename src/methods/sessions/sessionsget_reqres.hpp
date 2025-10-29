#pragma once

#include <string>

#include <libtorrent/settings_pack.hpp>

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
            int                       id;
            std::string               name;
            libtorrent::settings_pack settings;
        };

        Session session;
    };
}
