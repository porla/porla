#pragma once

#include <map>

#include <libtorrent/torrent_status.hpp>
#include <sqlite3.h>

namespace porla
{
    class TorrentsVTable
    {
    public:
        static int Install(sqlite3* db, std::map<libtorrent::info_hash_t, libtorrent::torrent_status>& torrents);
    };
}
