#include "sessionssettingsget.hpp"

#include "../../sessions.hpp"

using porla::Methods::Sessions::SessionsSettingsGet;
using porla::Methods::Sessions::SessionsSettingsGetReq;
using porla::Methods::Sessions::SessionsSettingsGetRes;

SessionsSettingsGet::SessionsSettingsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsSettingsGet::Invoke(const SessionsSettingsGetReq &req, WriteCb<SessionsSettingsGetRes> cb)
{
    const auto& state = m_sessions.Get(req.id);

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    cb.Ok(SessionsSettingsGetRes{
        .settings = state->session->get_settings()
    });
}
