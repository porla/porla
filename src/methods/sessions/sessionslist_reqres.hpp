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
            std::optional<std::map<std::string, nlohmann::json>> metadata;
            int                                                  torrents_total;
        };

        std::vector<Item> sessions;
    };
}
