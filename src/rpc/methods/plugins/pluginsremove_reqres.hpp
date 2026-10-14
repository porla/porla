#pragma once

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsRemoveReq
    {
        int id;
    };

    PORLA_JSON_REQUEST(PluginsRemoveReq, id)
}
