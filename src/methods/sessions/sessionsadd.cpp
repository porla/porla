#include "sessionsadd.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../data/models/sessions.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Methods::Sessions::SessionsAdd;
using porla::Methods::Sessions::SessionsAddReq;
using porla::Methods::Sessions::SessionsAddRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsAdd::SessionsAdd(sqlite3* db, porla::Sessions& sessions)
    : m_db(db),
    m_sessions(sessions)
{
}

void SessionsAdd::Invoke(const SessionsAddReq& req, WriteCb<SessionsAddRes> cb)
{
    std::string settings_base = req.settings_base.value_or("default");

    lt::settings_pack settings;
    if (settings_base == "default")               settings = lt::default_settings();
    if (settings_base == "min_memory_usage")      settings = lt::min_memory_usage();
    if (settings_base == "high_performance_seed") settings = lt::high_performance_seed();

    LibtorrentSettingsPack::Update(settings, req.settings.value_or(std::map<std::string, json>()));

    const auto session = Data::Models::Sessions::Session{
        .id                    = -1,
        .name                  = req.name,
        .is_default            = false,
        .metadata              = req.metadata.value_or({}),
        .params                = lt::session_params(settings),
        .timer_dht_stats       = req.timer_dht_stats.value_or(5000),
        .timer_save_state      = req.timer_save_state.value_or(300000),
        .timer_session_stats   = req.timer_session_stats.value_or(5000),
        .timer_torrent_updates = req.timer_torrent_updates.value_or(1000)
    };

    int session_id = porla::Data::Models::Sessions::Insert(
        m_db,
        session);

    BOOST_LOG_TRIVIAL(info) << "Session " << req.name << " added - loading...";

    m_sessions.LoadById(session_id);

    cb.Ok(SessionsAddRes{
        .id = session_id
    });
}
