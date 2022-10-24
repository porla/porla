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
            int download_rate;
            std::uint64_t flags;
            lt::info_hash_t info_hashes;
            int list_peers;
            int list_seeds;
            std::string name;
            int num_peers;
            int num_seeds;
            float progress;
            int queue_position;
            std::string save_path;
            std::int64_t size;
            int state;
            std::int64_t total;
            std::int64_t total_done;
            int upload_rate;
        };

        std::vector<Item> torrents;
        int torrents_total;
    };
}
