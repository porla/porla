#include "sessionspause.hpp"

#include "../../sessions.hpp"

using porla::Methods::SessionsPause;
using porla::Methods::SessionsPauseReq;
using porla::Methods::SessionsPauseRes;

SessionsPause::SessionsPause(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsPause::Invoke(const SessionsPauseReq& req, WriteCb<SessionsPauseRes> cb)
{
    const auto& state = req.name.has_value()
        ? m_sessions.Get(req.name.value())
        : m_sessions.Default();

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    state->session->pause();

    return cb.Ok(SessionsPauseRes{});
}
