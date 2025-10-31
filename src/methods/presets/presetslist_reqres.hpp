#pragma once

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace porla::Methods::Presets
{
    struct PresetsListReq {};

    struct PresetsListRes
    {
        struct ListItem
        {
            int                                   id;
            std::string                           name;
            bool                                  is_default;
            std::map<std::string, nlohmann::json> metadata;
        };

        std::vector<ListItem> presets;
    };
}