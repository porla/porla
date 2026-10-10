#include "sessionsremove.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsRemove;
using porla::Rpc::Methods::Sessions::SessionsRemoveReq;
using porla::Rpc::Methods::Sessions::SessionsRemoveRes;

SessionsRemove::SessionsRemove(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsRemove::Execute(const SessionsRemoveReq& req, ResponseWriterHandle cb)
{
    const auto session = m_sessions.Find(req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    if (session->is_default)
    {
        return cb->Error(-2, "Cannot remove default session");
    }

    const auto session_state = m_sessions.Get(session->id);

    if (session_state != nullptr && session_state->Count() > 0)
    {
        return cb->Error(-3, "Cannot remove session with torrents");
    }

    m_sessions.Remove(session->id);

    cb->Ok(SessionsRemoveRes{});
}
