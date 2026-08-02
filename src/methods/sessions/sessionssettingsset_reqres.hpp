#pragma once

#include <libtorrent/settings_pack.hpp>

namespace porla::Methods::Sessions
{
    struct SessionsSettingsSetReq
    {
        int id;
        libtorrent::settings_pack settings;
    };

    struct SessionsSettingsSetRes
    {
    };
}
