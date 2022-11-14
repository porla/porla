#pragma once

#include <libtorrent/session_params.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class SessionParams
    {
    public:
        static libtorrent::session_params GetLatest(sqlite3* db);
        static void Insert(sqlite3* db, libtorrent::session_params const& params);
    };
}