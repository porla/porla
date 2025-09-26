#include "sessionsresume.hpp"

#include "../../sessions.hpp"

using porla::Methods::SessionsResume;
using porla::Methods::SessionsResumeReq;
using porla::Methods::SessionsResumeRes;

SessionsResume::SessionsResume(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsResume::Invoke(const SessionsResumeReq& req, WriteCb<SessionsResumeRes> cb)
{
    const auto& state = m_sessions.Get(req.id);

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    state->session->resume();

    cb.Ok(SessionsResumeRes{});
}
