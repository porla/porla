#include "sessionssettingsset.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../data/models/sessions.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Methods::Sessions::SessionsSettingsSet;
using porla::Methods::Sessions::SessionsSettingsSetReq;
using porla::Methods::Sessions::SessionsSettingsSetRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsSettingsSet::SessionsSettingsSet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db),
    m_sessions(sessions)
{
}

void SessionsSettingsSet::Invoke(const SessionsSettingsSetReq& req, WriteCb<SessionsSettingsSetRes> cb)
{
    const auto& state = m_sessions.Get(req.id);

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    lt::settings_pack session_settings = state->session->get_settings();

    LibtorrentSettingsPack::Update(session_settings, req.settings);
    LibtorrentSettingsPack::UpdateStatic(session_settings);

    porla::Data::Models::Sessions::Update(
        m_db,
        req.id,
        session_settings);

    state->session->apply_settings(session_settings);

    BOOST_LOG_TRIVIAL(info) << "Session settings for " << state->name << " updated";

    cb.Ok(SessionsSettingsSetRes{});
}
