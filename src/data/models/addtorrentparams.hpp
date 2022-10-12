#pragma once

#include <libtorrent/add_torrent_params.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    struct AddTorrentParams
    {
        std::string name;
        libtorrent::add_torrent_params params;
        int queue_position;
        std::string save_path;

        static int Count(sqlite3* db);
        static void ForEach(sqlite3* db, const std::function<void(libtorrent::add_torrent_params&)>& cb);
        static void Insert(sqlite3* db, const libtorrent::info_hash_t& hash, const AddTorrentParams& params);
        static void Update(sqlite3* db, const libtorrent::info_hash_t& hash, const AddTorrentParams& params);
    };
}
