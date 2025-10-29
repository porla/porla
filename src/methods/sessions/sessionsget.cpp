#include "sessionsget.hpp"

#include "../../sessions.hpp"

using porla::Methods::Sessions::SessionsGet;
using porla::Methods::Sessions::SessionsGetReq;
using porla::Methods::Sessions::SessionsGetRes;

SessionsGet::SessionsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsGet::Invoke(const SessionsGetReq &req, WriteCb<SessionsGetRes> cb)
{
    SessionsGetRes res;

    const auto& state = m_sessions.Get(req.id);

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    cb.Ok(SessionsGetRes{
        .session = SessionsGetRes::Session{
            .id       = state->id,
            .name     = state->name,
            .settings = state->session->get_settings()
        }
    });
}
