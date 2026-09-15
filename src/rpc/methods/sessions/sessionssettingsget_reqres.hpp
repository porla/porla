#pragma once

#include <libtorrent/settings_pack.hpp>

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsSettingsGetReq
    {
        int id;
    };

    struct SessionsSettingsGetRes
    {
        libtorrent::settings_pack settings;
    };
}
