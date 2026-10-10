#include "sessionsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsUpdate;
using porla::Rpc::Methods::Sessions::SessionsUpdateReq;
using porla::Rpc::Methods::Sessions::SessionsUpdateRes;

SessionsUpdate::SessionsUpdate(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsUpdate::Execute(const SessionsUpdateReq& req, ResponseWriterHandle cb)
{
    auto session = m_sessions.Find(req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    session->is_default = req.is_default;
    session->metadata   = req.metadata;
    session->name       = req.name;

    m_sessions.Update(*session);

    cb->Ok(SessionsUpdateRes{});
}
