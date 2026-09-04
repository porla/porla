#pragma once

#include <libtorrent/add_torrent_params.hpp>
#include <sqlite3.h>

namespace porla
{
    class TorrentClientData;
}

namespace porla::Data::Models
{
    struct AddTorrentParams
    {
        static int Count(sqlite3* db, const int session);
        static void ForEach(sqlite3* db, const int session, const std::function<void(lt::add_torrent_params&)>& cb);
        static void Insert(sqlite3* db, const int session, const lt::info_hash_t& hash, const lt::add_torrent_params& params, const TorrentClientData* client_data, const int queue_pos);
        static void Remove(sqlite3* db, const int session, const lt::info_hash_t& hash);
        static void Update(sqlite3* db, const int session, const lt::info_hash_t& hash, const lt::add_torrent_params& params, const TorrentClientData* client_data, const int queue_pos);
    };
}
