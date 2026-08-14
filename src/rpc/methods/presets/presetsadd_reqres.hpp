#pragma once

#include <string>

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
}
