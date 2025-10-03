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
        "CREATE TABLE sessions ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL UNIQUE,"
        "params BLOB NULL,"
        "settings BLOB NOT NULL"
        ");",
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

    return res;
}