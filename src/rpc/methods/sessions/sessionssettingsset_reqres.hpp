#pragma once

#include <libtorrent/settings_pack.hpp>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsSettingsSetReq
    {
        int id;
        libtorrent::settings_pack settings;
    };

    PORLA_JSON_REQUEST(SessionsSettingsSetReq, id, settings)
}
