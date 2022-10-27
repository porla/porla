#include "sessionpause.hpp"

#include "../session.hpp"

using porla::Methods::SessionPause;
using porla::Methods::SessionPauseReq;
using porla::Methods::SessionPauseRes;

SessionPause::SessionPause(porla::ISession &session)
    : m_session(session)
{
}

void SessionPause::Invoke(const SessionPauseReq& req, WriteCb<SessionPauseRes> cb)
{
    m_session.Pause();
}
