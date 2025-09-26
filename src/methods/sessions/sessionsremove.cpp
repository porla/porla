#include "sessionsremove.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../data/models/sessions.hpp"

using porla::Methods::Sessions::SessionsRemove;
using porla::Methods::Sessions::SessionsRemoveReq;
using porla::Methods::Sessions::SessionsRemoveRes;

SessionsRemove::SessionsRemove(sqlite3* db, porla::Sessions& sessions)
    : m_db(db),
    m_sessions(sessions)
{
}

void SessionsRemove::Invoke(const SessionsRemoveReq& req, WriteCb<SessionsRemoveRes> cb)
{
    const auto& session = m_sessions.Get(req.id);

    if (session == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    if (session->name == "default")
    {
        return cb.Error(-2, "Cannot remove default session");
    }

    m_sessions.UnloadByName(session->name);

    porla::Data::Models::Sessions::Remove(m_db, session->name);

    BOOST_LOG_TRIVIAL(info) << "Session " << session->name << " removed";

    cb.Ok(SessionsRemoveRes{});
}
