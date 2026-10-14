#pragma once

#include <map>
#include <string>
#include <vector>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Presets
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

    PORLA_JSON_NO_PARAMS(PresetsListReq)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        PresetsListRes::ListItem,
        id,
        name,
        is_default,
        metadata)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        PresetsListRes,
        presets)
}