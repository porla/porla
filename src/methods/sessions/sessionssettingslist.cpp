#include "sessionssettingslist.hpp"

#include "../../sessions.hpp"

using porla::Methods::SessionsSettingsList;
using porla::Methods::SessionsSettingsListReq;
using porla::Methods::SessionsSettingsListRes;

SessionsSettingsList::SessionsSettingsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsSettingsList::Invoke(const SessionsSettingsListReq &req, WriteCb<SessionsSettingsListRes> cb)
{
    SessionsSettingsListRes res;

    const auto& state = req.name.has_value()
        ? m_sessions.Get(req.name.value())
        : m_sessions.Default();

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    cb.Ok(SessionsSettingsListRes{
        .settings = state->session->get_settings()
    });
}
