#pragma once

#include <optional>
#include <string>

#include <libtorrent/session_params.hpp>
#include <libtorrent/settings_pack.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class Sessions
    {
    public:
        struct Session
        {
            int                        id;
            std::string                name;
            libtorrent::session_params params;
        };

        static void ForEach(sqlite3* db, const std::function<void(const Session&)>& cb);
        static std::optional<Session> GetById(sqlite3* db, int id);
        static void Insert(sqlite3* db, const std::string& name, const libtorrent::settings_pack& settings);
        static void Remove(sqlite3* db, int id);
        static void Update(sqlite3* db, int id, const libtorrent::session_params& params);
        static void Update(sqlite3* db, int id, const libtorrent::settings_pack& settings);
    };
}
