#pragma once

#include <map>
#include <optional>
#include <string>

#include <libtorrent/session_params.hpp>
#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

namespace porla::Data::Models
{
    class Sessions
    {
    public:
        struct Session
        {
            int                                   id;
            std::string                           name;
            bool                                  is_default;
            std::map<std::string, nlohmann::json> metadata;
            libtorrent::session_params            params;
            int                                   timer_dht_stats;
            int                                   timer_save_state;
            int                                   timer_session_stats;
            int                                   timer_torrent_updates;
        };

        static std::optional<Session> GetDefault(sqlite3* db);
        static std::optional<Session> GetById(sqlite3* db, int id);
        static int Insert(sqlite3* db, const Session& session);
        static std::vector<Session> List(sqlite3* db);
        static void Remove(sqlite3* db, int id);
        static void SetDefault(sqlite3* db, int id);
        static void Update(sqlite3* db, const Session& session);
    };
}
