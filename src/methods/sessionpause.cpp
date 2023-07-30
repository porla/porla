#include "sessionpause.hpp"

#include "../sessions.hpp"

using porla::Methods::SessionPause;
using porla::Methods::SessionPauseReq;
using porla::Methods::SessionPauseRes;

SessionPause::SessionPause(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionPause::Invoke(const SessionPauseReq& req, WriteCb<SessionPauseRes> cb)
{
    m_sessions.Default()->session->pause();
    return cb.Ok(SessionPauseRes{});
}
