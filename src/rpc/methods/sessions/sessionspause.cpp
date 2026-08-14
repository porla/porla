#include "sessionspause.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsPause;
using porla::Rpc::Methods::Sessions::SessionsPauseReq;
using porla::Rpc::Methods::Sessions::SessionsPauseRes;

SessionsPause::SessionsPause(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsPause::Execute(const SessionsPauseReq& req, ResponseWriterHandle cb)
{
    const auto session = Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& state = m_sessions.Get(session->id);

    if (state == nullptr)
    {
        return cb->Error(-2, "Session not loaded");
    }

    state->session->pause();

    return cb->Ok(SessionsPauseRes{});
}
