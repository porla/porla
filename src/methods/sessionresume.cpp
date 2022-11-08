#include "sessionresume.hpp"

#include "../session.hpp"

using porla::Methods::SessionResume;
using porla::Methods::SessionResumeReq;
using porla::Methods::SessionResumeRes;

SessionResume::SessionResume(porla::ISession& session)
    : m_session(session)
{
}

void SessionResume::Invoke(const SessionResumeReq& req, WriteCb<SessionResumeRes> cb)
{
    m_session.Resume();
    cb.Ok(SessionResumeRes{});
}
