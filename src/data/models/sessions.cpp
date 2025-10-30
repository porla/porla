#include "sessions.hpp"

#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>
#include <libtorrent/bdecode.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>

#include "../../buildinfo.hpp"
#include "../statement.hpp"
#include "../../utils/ltsettings.hpp"

using porla::BuildInfo;
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
        settings,
        timer_dht_stats,
        timer_save_state,
        timer_session_stats,
        timer_torrent_updates
    FROM sessions
)sql";

static std::optional<Sessions::Session> LoadSessionFromRow(const Statement::IRow& row)
{
    std::vector<char> metadata_buffer = row.GetBuffer("metadata");
    std::vector<char> settings_buffer = row.GetBuffer("settings");

    lt::error_code ec;
    lt::bdecode_node node = lt::bdecode(settings_buffer, ec);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to bdecode settings for session " << row.GetStdString("name") << ": " << ec.message();
        return std::nullopt;
    }

    std::vector<char> params_buffer = row.GetBuffer("params");
    lt::session_params params = params_buffer.size() > 0
        ? lt::read_session_params(params_buffer, lt::session::save_dht_state)
        : lt::session_params();

    params.settings = lt::load_pack_from_dict(node);
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

void Sessions::ForEach(sqlite3 *db, const std::function<void(const Sessions::Session&)>& cb)
{
    auto stmt = Statement::Prepare(db, SessionsSelectPrefix + " ORDER BY name ASC");
    stmt.Step(
        [&cb](const Statement::IRow& row)
        {
            auto s = LoadSessionFromRow(row);

            if (s.has_value())
            {
                cb(s.value());
            }

            return SQLITE_OK;
        });
}

std::optional<Sessions::Session> Sessions::GetById(sqlite3* db, int id)
{
    std::optional<Sessions::Session> session;

    Statement::Prepare(db, SessionsSelectPrefix + " WHERE id = $1")
        .Bind(1, id)
        .Step(
            [&session](auto const& row)
            {
                session = LoadSessionFromRow(row);
                return SQLITE_OK;
            });

    return session;
}

std::optional<Sessions::Session> Sessions::GetByName(sqlite3* db, const std::string& name)
{
    std::optional<Sessions::Session> session;

    Statement::Prepare(db, SessionsSelectPrefix + " WHERE name = $1")
        .Bind(1, std::string_view(name))
        .Step(
            [&session](auto const& row)
            {
                session = LoadSessionFromRow(row);
                return SQLITE_OK;
            });

    return session;
}

int Sessions::Insert(sqlite3* db, const std::string& name, const lt::settings_pack& settings)
{
    lt::entry::dictionary_type dict;
    lt::save_settings_to_dict(settings, dict);

    std::vector<char> buf;
    lt::bencode(std::back_inserter(buf), dict);

    auto stmt = Statement::Prepare(db, "INSERT INTO sessions (name, settings) VALUES ($1, $2);");
    stmt.Bind(1, std::string_view(name));
    stmt.Bind(2, buf);
    stmt.Execute();

    return sqlite3_last_insert_rowid(db);
}

void Sessions::Remove(sqlite3* db, int id)
{
    auto stmt = Statement::Prepare(db, "DELETE FROM sessions WHERE id = $1");
    stmt.Bind(1, id);
    stmt.Execute();
}

void Sessions::SetDefault(sqlite3* db, int id)
{
    Statement::Prepare(db, "UPDATE sessions SET is_default = 0")
        .Execute();

    Statement::Prepare(db, "UPDATE sessions SET is_default = 1 WHERE id = ?")
        .Bind(1, id)
        .Execute();
}

void Sessions::Update(sqlite3* db, int id, const std::map<std::string, nlohmann::json>& metadata)
{
    const auto json = nlohmann::json(metadata).dump();

    auto stmt = Statement::Prepare(db, "UPDATE sessions SET metadata = $1 WHERE id = $2");
    stmt.Bind(1, std::string_view(json));
    stmt.Bind(2, id);
    stmt.Execute();
}

void Sessions::Update(sqlite3* db, int id, const lt::session_params& params)
{
    std::vector params_buffer = lt::write_session_params_buf(
        params,
        lt::session::save_dht_state
        | lt::session::save_extension_state);

    lt::entry::dictionary_type dict;
    lt::save_settings_to_dict(params.settings, dict);

    std::vector<char> settings_buffer;
    lt::bencode(std::back_inserter(settings_buffer), dict);

    auto stmt = Statement::Prepare(db, "UPDATE sessions SET params = $1, settings = $2 WHERE id = $3");
    stmt.Bind(1, params_buffer);
    stmt.Bind(2, settings_buffer);
    stmt.Bind(3, id);
    stmt.Execute();
}

void Sessions::Update(sqlite3* db, int id, const lt::settings_pack& settings)
{
    lt::entry::dictionary_type dict;
    lt::save_settings_to_dict(settings, dict);

    std::vector<char> settings_buffer;
    lt::bencode(std::back_inserter(settings_buffer), dict);

    auto stmt = Statement::Prepare(db, "UPDATE sessions SET settings = $1 WHERE id = $2");
    stmt.Bind(1, settings_buffer);
    stmt.Bind(2, id);
    stmt.Execute();
}
