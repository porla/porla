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
            std::string name;
            bool        is_dht_running;
            bool        is_listening;
            bool        is_paused;
            int         torrents_total;
        };

        std::vector<Item> sessions;
    };
}
