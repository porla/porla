#pragma once

#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Presets
{
    struct PresetsAddReq
    {
        std::string name;
    };

    struct PresetsAddRes
    {
        int id;
    };

    PORLA_JSON_REQUEST(PresetsAddReq, name)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(PresetsAddRes, id)
}
