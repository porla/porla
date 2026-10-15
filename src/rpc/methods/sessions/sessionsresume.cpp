#include "sessionsresume.hpp"

#include "../resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsResume;

SessionsResume::SessionsResume(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsResume::Execute(const SessionRef& req, ResponseWriterHandle cb)
{
    const auto session = ResolveSession(m_sessions, req.id, cb);

    if (!session)
    {
        return;
    }

    session->session->Libtorrent().resume();

    cb->Ok();
}
