#include "sessions.hpp"

#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>
#include <libtorrent/bdecode.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>

#include "../statement.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Data::Statement;
using porla::Data::Models::Sessions;
using porla::Utils::LibtorrentSettingsPack;

namespace lt = libtorrent;

const std::string SessionsSelectPrefix = R"sql(
    SELECT
        id,
        name,
        is_default,
        metadata,
        params,
        timer_dht_stats,
        timer_save_state,
        timer_session_stats,
        timer_torrent_updates
    FROM sessions
)sql";

static Sessions::Session LoadSessionFromRow(const Statement::IRow& row)
{
    std::vector<char> metadata_buffer = row.GetBuffer("metadata");
    std::vector<char> params_buffer = row.GetBuffer("params");

    lt::session_params params = params_buffer.size() > 0
        ? lt::read_session_params(params_buffer)
        : lt::session_params();

    LibtorrentSettingsPack::UpdateStatic(params.settings);

    return Sessions::Session{
        .id                    = row.GetInt32("id"),
        .name                  = row.GetStdString("name"),
        .is_default            = row.GetInt32("is_default") == 1,
        .metadata              = metadata_buffer.empty()
                                     ? std::map<std::string, nlohmann::json>()
                                     : nlohmann::json::parse(metadata_buffer).get<std::map<std::string, nlohmann::json>>(),
        .params                = params,
        .timer_dht_stats       = row.GetInt32("timer_dht_stats"),
        .timer_save_state      = row.GetInt32("timer_save_state"),
        .timer_session_stats   = row.GetInt32("timer_session_stats"),
        .timer_torrent_updates = row.GetInt32("timer_torrent_updates")
    };
}

std::optional<Sessions::Session> Sessions::GetDefault(sqlite3* db)
{
    std::optional<Sessions::Session> session;

    Statement::Prepare(db, SessionsSelectPrefix + " WHERE is_default = 1")
        .Step(
            [&session](auto const& row)
            {
                session = LoadSessionFromRow(row);
                return SQLITE_OK;
            });

    return session;
}

std::optional<Sessions::Session> Sessions::GetById(sqlite3* db, int id)
{
    std::optional<Sessions::Session> session;

    Statement::Prepare(db, SessionsSelectPrefix + " WHERE id = $id")
        .Bind("$id", id)
        .Step(
            [&session](auto const& row)
            {
                session = LoadSessionFromRow(row);
                return SQLITE_OK;
            });

    return session;
}

int Sessions::Insert(sqlite3* db, const Sessions::Session& session)
{
    std::vector params_buffer = lt::write_session_params_buf(session.params);

    auto stmt = Statement::Prepare(db, "INSERT INTO sessions (name, params) VALUES ($name, $params);");
    stmt.Bind("$name",   session.name);
    stmt.Bind("$params", params_buffer);
    stmt.Execute();

    return sqlite3_last_insert_rowid(db);
}

std::vector<Sessions::Session> Sessions::List(sqlite3* db)
{
    std::vector<Sessions::Session> sessions;

    Statement::Prepare(db, SessionsSelectPrefix + " ORDER BY name ASC")
        .Step(
            [&sessions](const auto& row)
            {
                sessions.emplace_back(LoadSessionFromRow(row));
                return SQLITE_OK;
            });

    return sessions;
}

void Sessions::Remove(sqlite3* db, int id)
{
    auto stmt = Statement::Prepare(db, "DELETE FROM sessions WHERE id = $id");
    stmt.Bind("$id", id);
    stmt.Execute();
}

void Sessions::SetDefault(sqlite3* db, int id)
{
    Statement::Prepare(db, "UPDATE sessions SET is_default = 0")
        .Execute();

    Statement::Prepare(db, "UPDATE sessions SET is_default = 1 WHERE id = $id")
        .Bind("$id", id)
        .Execute();
}

void Sessions::Update(sqlite3* db, const Sessions::Session& session)
{
    std::vector params_buffer = lt::write_session_params_buf(session.params);

    const auto json = nlohmann::json(session.metadata).dump();

    auto stmt = Statement::Prepare(db, "UPDATE sessions SET metadata = $metadata, name = $name, params = $params WHERE id = $id");
    stmt.Bind("$id",       session.id);
    stmt.Bind("$metadata", json);
    stmt.Bind("$name",     session.name);
    stmt.Bind("$params",   params_buffer);
    stmt.Execute();
}
