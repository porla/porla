#pragma once

#include <libtorrent/add_torrent_params.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class AddTorrentParams
    {
    public:
        static int Count(sqlite3* db);
        static void ForEach(sqlite3* db, const std::function<void(const libtorrent::add_torrent_params&)>& cb);
    };
}
