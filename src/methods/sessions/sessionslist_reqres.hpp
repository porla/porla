#pragma once

#include <string>
#include <vector>

namespace porla::Methods
{
    struct SessionsListReq
    {
    };

    struct SessionsListRes
    {
        struct Item
        {
            int                                                  id;
            std::string                                          name;
            bool                                                 is_default;
            bool                                                 is_listening;
            bool                                                 is_paused;
            std::optional<std::map<std::string, nlohmann::json>> metadata;
            int                                                  torrents_total;
        };

        std::vector<Item> sessions;
    };
}
