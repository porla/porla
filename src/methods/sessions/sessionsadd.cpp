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
    const auto existing_session = porla::Data::Models::Sessions::GetByName(m_db, req.name);

    if (existing_session)
    {
        return cb.Error(-1, "Session with specified name already exists");
    }

    std::string settings_base = req.settings_base.value_or("default");

    lt::settings_pack settings;
    if (settings_base == "default")               settings = lt::default_settings();
    if (settings_base == "min_memory_usage")      settings = lt::min_memory_usage();
    if (settings_base == "high_performance_seed") settings = lt::high_performance_seed();

    LibtorrentSettingsPack::Update(settings, req.settings.value_or(std::map<std::string, json>()));

    porla::Data::Models::Sessions::Insert(
        m_db,
        req.name,
        settings);

    BOOST_LOG_TRIVIAL(info) << "Session " << req.name << " added - loading...";

    m_sessions.LoadByName(req.name);

    cb.Ok(SessionsAddRes{});
}
