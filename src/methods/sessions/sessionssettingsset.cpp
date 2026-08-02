#include "sessionssettingsset.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Methods::Sessions::SessionsSettingsSet;
using porla::Methods::Sessions::SessionsSettingsSetReq;
using porla::Methods::Sessions::SessionsSettingsSetRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsSettingsSet::SessionsSettingsSet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsSettingsSet::Invoke(const SessionsSettingsSetReq &req, WriteCb<SessionsSettingsSetRes> cb)
{
    const auto& state = m_sessions.Get(req.id);

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    lt::settings_pack sp = req.settings;

    LibtorrentSettingsPack::UpdateStatic(sp);

    state->session->apply_settings(sp);

    BOOST_LOG_TRIVIAL(info) << "Session settings for " << state->name << " updated";

    cb.Ok(SessionsSettingsSetRes{});
}
