#include "sessionresume.hpp"

#include "../sessions.hpp"

using porla::Methods::SessionResume;
using porla::Methods::SessionResumeReq;
using porla::Methods::SessionResumeRes;

SessionResume::SessionResume(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionResume::Invoke(const SessionResumeReq& req, WriteCb<SessionResumeRes> cb)
{
    m_sessions.Default()->session->resume();
    cb.Ok(SessionResumeRes{});
}
