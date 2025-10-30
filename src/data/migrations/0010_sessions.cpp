#include "0010_sessions.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>

#include "../../config.hpp"
#include "../statement.hpp"

using json = nlohmann::json;
using porla::Data::Migrations::Sessions;

namespace lt = libtorrent;

int Sessions::Migrate(sqlite3 *db, const std::unique_ptr<porla::Config> &cfg)
{
    BOOST_LOG_TRIVIAL(info) << "Adding 'sessions' table";

    int res = sqlite3_exec(
        db,
        R"sql(
        CREATE TABLE sessions (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL UNIQUE,
            is_default INTEGER NOT NULL DEFAULT 0,
            metadata BLOB NULL,
            params BLOB NULL,
            settings BLOB NOT NULL,
            timer_dht_stats INTEGER NOT NULL DEFAULT 5000,
            timer_save_state INTEGER NOT NULL DEFAULT 300000,
            timer_session_stats INTEGER NOT NULL DEFAULT 5000,
            timer_torrent_updates INTEGER NOT NULL DEFAULT 1000,
            CHECK (is_default IN (0, 1))
        );

        CREATE UNIQUE INDEX uq_sessions_default
        ON sessions(is_default)
        WHERE is_default = 1;
        )sql",
        nullptr,
        nullptr,
        nullptr);

    if (res != SQLITE_OK)
    {
        return res;
    }

    for (const auto& [name, settings] : cfg->sessions)
    {
        BOOST_LOG_TRIVIAL(info) << "Inserting session " << name;

        lt::entry::dictionary_type dict;
        lt::save_settings_to_dict(settings, dict);

        std::vector<char> buf;
        lt::bencode(std::back_inserter(buf), dict);

        auto stmt = Statement::Prepare(db, "INSERT INTO sessions (name, settings) VALUES ($1, $2);");
        stmt.Bind(1, std::string_view(name));
        stmt.Bind(2, buf);
        stmt.Execute();
    }

    res = sqlite3_exec(
        db,
        "UPDATE sessions SET is_default = 1 WHERE name = 'default'",
        nullptr,
        nullptr,
        nullptr);

    return res;
}