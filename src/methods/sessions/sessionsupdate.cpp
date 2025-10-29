#include "sessionsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../data/models/sessions.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Methods::Sessions::SessionsUpdate;
using porla::Methods::Sessions::SessionsUpdateReq;
using porla::Methods::Sessions::SessionsUpdateRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsUpdate::SessionsUpdate(sqlite3* db, porla::Sessions& sessions)
    : m_db(db),
    m_sessions(sessions)
{
}

void SessionsUpdate::Invoke(const SessionsUpdateReq& req, WriteCb<SessionsUpdateRes> cb)
{
    const auto& state = m_sessions.Get(req.id);

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    if (req.settings)
    {
        lt::settings_pack session_settings = state->session->get_settings();

        LibtorrentSettingsPack::Update(session_settings, req.settings.value());
        LibtorrentSettingsPack::UpdateStatic(session_settings);

        porla::Data::Models::Sessions::Update(
            m_db,
            req.id,
            session_settings);

        state->session->apply_settings(session_settings);

        BOOST_LOG_TRIVIAL(info) << "Session settings for " << state->name << " updated";
    }

    cb.Ok(SessionsUpdateRes{});
}
