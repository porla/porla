#pragma once

#include <libtorrent/add_torrent_params.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class AddTorrentParams
    {
    public:
        static int Count(sqlite3* db);
        static void ForEach(sqlite3* db, const std::function<void(libtorrent::add_torrent_params&)>& cb);
        static void Insert(sqlite3* db, const libtorrent::add_torrent_params& params, int pos);
        static void Update(sqlite3* db, const libtorrent::add_torrent_params& params);
    };
}
