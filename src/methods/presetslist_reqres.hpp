#pragma once

#include <map>
#include <string>

#include "../config.hpp"

namespace porla::Methods
{
    struct PresetsListReq {};

    struct PresetsListRes
    {
        std::map<std::string, Config::Preset> presets;
    };
}