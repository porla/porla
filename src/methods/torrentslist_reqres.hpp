#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsListReq
    {
        std::optional<int> page;
        std::optional<int> page_size;
        std::optional<std::string> order_by;
        std::optional<std::string> order_by_dir;
    };

    struct TorrentsListRes
    {
        struct Item
        {
            std::string name;
            int queue_pos;
        };

        std::vector<Item> torrents;
        int torrents_total;
    };
}
