#pragma once

#include <string>
#include <vector>

namespace porla::Methods::Presets
{
    struct PresetsListReq {};

    struct PresetsListRes
    {
        struct ListItem
        {
            int         id;
            std::string name;
        };

        std::vector<ListItem> presets;
    };
}