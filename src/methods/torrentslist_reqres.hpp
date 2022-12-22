#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsListReqFilter
    {
        std::string field;
        json args;
    };

    struct TorrentsListReq
    {
        std::optional<std::vector<TorrentsListReqFilter>> filters;
        std::optional<std::vector<std::string>> include_metadata;
        std::optional<int> page;
        std::optional<int> page_size;
        std::optional<std::string> order_by;
        std::optional<std::string> order_by_dir;
    };

    struct TorrentsListRes
    {
        struct Item
        {
            std::int64_t all_time_download;
            std::int64_t all_time_upload;
            json category; // TODO: Make this data type better. It is not representing a string, or null.
            int download_rate;
            libtorrent::error_code error;
            std::int64_t eta;
            std::uint64_t flags;
            lt::info_hash_t info_hash;
            int list_peers;
            int list_seeds;
            std::optional<json> metadata;
            bool moving_storage;
            std::string name;
            int num_peers;
            int num_seeds;
            float progress;
            int queue_position;
            double ratio;
            std::string save_path;
            std::int64_t size;
            int state;
            std::vector<std::string> tags;
            std::int64_t total;
            std::int64_t total_done;
            int upload_rate;
        };

        int page;
        int page_size;

        std::vector<Item> torrents;
        int torrents_total;
    };
}
