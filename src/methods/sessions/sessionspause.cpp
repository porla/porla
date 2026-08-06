#include "sessionspause.hpp"

#include "../../data/models/sessions.hpp"
#include "../../sessions.hpp"

using porla::Methods::SessionsPause;
using porla::Methods::SessionsPauseReq;
using porla::Methods::SessionsPauseRes;

SessionsPause::SessionsPause(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsPause::Invoke(const SessionsPauseReq& req, WriteCb<SessionsPauseRes> cb)
{
    const auto session = Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb.Error(-1, "Session not found");
    }

    const auto& state = m_sessions.Get(session->id);

    if (state == nullptr)
    {
        return cb.Error(-2, "Session not loaded");
    }

    state->session->pause();

    return cb.Ok(SessionsPauseRes{});
}
