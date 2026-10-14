#pragma once

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Presets
{
    struct PresetsRemoveReq
    {
        int id;
    };

    PORLA_JSON_REQUEST(PresetsRemoveReq, id)
}
