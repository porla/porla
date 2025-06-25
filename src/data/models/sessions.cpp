#include "sessions.hpp"

#include <boost/log/trivial.hpp>

#include <libtorrent/alert_types.hpp>
#include <libtorrent/bdecode.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>

#include "../../buildinfo.hpp"
#include "../statement.hpp"

using porla::BuildInfo;
using porla::Data::Statement;
using porla::Data::Models::Sessions;

namespace lt = libtorrent;

static std::optional<Sessions::Session> LoadSessionFromRow(const Statement::IRow& row)
{
    std::vector<char> settings_buffer = row.GetBuffer(2);
    std::string bd(settings_buffer.begin(), settings_buffer.end());

    lt::error_code ec;
    lt::bdecode_node node = lt::bdecode(bd, ec);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to bdecode settings for session " << row.GetStdString(0) << ": " << ec.message();
        return std::nullopt;
    }

    std::vector<char> params_buffer = row.GetBuffer(1);
    lt::session_params params = params_buffer.size() > 0
        ? lt::read_session_params(params_buffer, lt::session::save_dht_state)
        : lt::session_params();

    params.settings = lt::load_pack_from_dict(node);

    Sessions::Session s;
    s.name = row.GetStdString(0);
    s.params = params;

    lt::alert_category_t alerts =
        lt::alert::status_notification
        | lt::alert::storage_notification
        | lt::alert::tracker_notification;

    // These settings are static - i.e even if one sets them to something dumb, we
    // overwrite the settings with these values.

    std::stringstream user_agent;
    user_agent << "porla/" << BuildInfo::Version() << " libtorrent/" << LIBTORRENT_VERSION;

    s.params.settings.set_int(lt::settings_pack::alert_mask, alerts);
    s.params.settings.set_str(
        lt::settings_pack::peer_fingerprint,
        lt::generate_fingerprint("PO", BuildInfo::VersionMajor(), BuildInfo::VersionMinor(), BuildInfo::VersionPatch()));
    s.params.settings.set_str(lt::settings_pack::user_agent, user_agent.str());

    return s;
}

void Sessions::ForEach(sqlite3 *db, const std::function<void(const Sessions::Session&)>& cb)
{
    auto stmt = Statement::Prepare(db, "SELECT name,params,settings FROM sessions");
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

std::optional<Sessions::Session> Sessions::GetByName(sqlite3* db, const std::string& name)
{
    std::optional<Sessions::Session> session;

    Statement::Prepare(db, "SELECT name,params,settings FROM sessions WHERE name = $1")
        .Bind(1, std::string_view(name))
        .Step(
            [&session](auto const& row)
            {
                session = LoadSessionFromRow(row);
                return SQLITE_OK;
            });

    return session;
}

void Sessions::Insert(sqlite3* db, const std::string& name, const lt::settings_pack& settings)
{
    lt::entry::dictionary_type dict;
    lt::save_settings_to_dict(settings, dict);

    std::vector<char> buf;
    lt::bencode(std::back_inserter(buf), dict);

    auto stmt = Statement::Prepare(db, "INSERT INTO sessions (name, settings) VALUES ($1, $2);");
    stmt.Bind(1, std::string_view(name));
    stmt.Bind(2, buf);
    stmt.Execute();
}

void Sessions::Remove(sqlite3* db, const std::string& name)
{
    auto stmt = Statement::Prepare(db, "DELETE FROM sessions WHERE name = $1");
    stmt.Bind(1, std::string_view(name));
    stmt.Execute();
}

void Sessions::Update(sqlite3* db, const std::string& name, const lt::session_params& params)
{
    std::vector params_buffer = lt::write_session_params_buf(
        params,
        lt::session::save_dht_state
        | lt::session::save_extension_state);

    lt::entry::dictionary_type dict;
    lt::save_settings_to_dict(params.settings, dict);

    std::vector<char> settings_buffer;
    lt::bencode(std::back_inserter(settings_buffer), dict);

    auto stmt = Statement::Prepare(db, "UPDATE sessions SET params = $1, settings = $2 WHERE name = $3");
    stmt.Bind(1, params_buffer);
    stmt.Bind(2, settings_buffer);
    stmt.Bind(3, std::string_view(name));
    stmt.Execute();
}

void Sessions::Update(sqlite3* db, const std::string& name, const lt::settings_pack& settings)
{
    lt::entry::dictionary_type dict;
    lt::save_settings_to_dict(settings, dict);

    std::vector<char> settings_buffer;
    lt::bencode(std::back_inserter(settings_buffer), dict);

    auto stmt = Statement::Prepare(db, "UPDATE sessions SET settings = $1 WHERE name = $2");
    stmt.Bind(1, settings_buffer);
    stmt.Bind(2, std::string_view(name));
    stmt.Execute();
}
