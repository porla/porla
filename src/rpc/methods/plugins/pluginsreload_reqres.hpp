#pragma once

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsReloadReq
    {
        int id;
    };

    PORLA_JSON_REQUEST(PluginsReloadReq, id)
}
