#pragma once

#include <libtorrent/settings_pack.hpp>

#include "../../../json/utils.hpp"

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

    PORLA_JSON_REQUEST(SessionsSettingsGetReq, id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(SessionsSettingsGetRes, settings)
}
