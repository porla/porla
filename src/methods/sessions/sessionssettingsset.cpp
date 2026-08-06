#include "sessionssettingsset.hpp"

#include <boost/log/trivial.hpp>

#include "../../data/models/sessions.hpp"
#include "../../sessions.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Methods::Sessions::SessionsSettingsSet;
using porla::Methods::Sessions::SessionsSettingsSetReq;
using porla::Methods::Sessions::SessionsSettingsSetRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsSettingsSet::SessionsSettingsSet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsSettingsSet::Invoke(const SessionsSettingsSetReq &req, WriteCb<SessionsSettingsSetRes> cb)
{
    auto session = Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb.Error(-1, "Session not found");
    }

    session->params.settings = req.settings;

    LibtorrentSettingsPack::UpdateStatic(session->params.settings);

    Data::Models::Sessions::Update(
        m_db,
        *session);

    if (const auto& state = m_sessions.Get(session->id))
    {
        state->session->apply_settings(session->params.settings);
    }

    BOOST_LOG_TRIVIAL(info) << "Session settings for " << session->name << " updated";

    cb.Ok(SessionsSettingsSetRes{});
}
