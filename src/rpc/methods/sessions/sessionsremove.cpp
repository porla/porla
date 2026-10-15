#include "sessionsremove.hpp"

#include "../resolve.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsRemove;

SessionsRemove::SessionsRemove(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsRemove::Execute(const SessionRef& req, ResponseWriterHandle cb)
{
    const auto session = FindSession(m_sessions, req.id, cb);

    if (!session)
    {
        return;
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
