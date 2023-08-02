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
    const auto& state = req.name.has_value()
        ? m_sessions.Get(req.name.value())
        : m_sessions.Default();

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    state->session->resume();

    cb.Ok(SessionsResumeRes{});
}
