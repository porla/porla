#include "sessionsremove.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsRemove;
using porla::Rpc::Methods::Sessions::SessionsRemoveReq;

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

    if (m_sessions.CountParams(session->id) > 0 || (session_state != nullptr && !session_state->Empty()))
    {
        return cb->Error(-3, "Cannot remove session with torrents");
    }

    m_sessions.Remove(session->id);

    cb->Ok();
}
