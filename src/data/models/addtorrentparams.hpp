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
        struct Cursor
        {
            bool         unqueued = false;
            std::int64_t position = 0;
            std::int64_t id       = -1;
        };

        static int Count(sqlite3* db, const int session);
        static void Insert(sqlite3* db, const int session, const lt::info_hash_t& hash, const lt::add_torrent_params& params, const TorrentClientData& client_data, const int queue_pos);
        static bool Next(sqlite3* db, const int session, Cursor& cursor, const int max, const std::function<void(lt::add_torrent_params&)>& cb);
        static void Remove(sqlite3* db, const int session, const lt::info_hash_t& hash);
        static void Update(sqlite3* db, const int session, const lt::info_hash_t& hash, const lt::add_torrent_params& params, const int queue_pos);
        static void UpdateClientData(sqlite3* db, const int session, const lt::info_hash_t& hash, const TorrentClientData& client_data);
    };
}
