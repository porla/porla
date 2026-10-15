#include "sessionspause.hpp"

#include "../resolve.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsPause;

SessionsPause::SessionsPause(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsPause::Execute(const SessionRef& req, ResponseWriterHandle cb)
{
    const auto session = ResolveSession(m_sessions, req.id, cb);

    if (!session)
    {
        return;
    }

    session->session->Libtorrent().pause();

    return cb->Ok();
}
