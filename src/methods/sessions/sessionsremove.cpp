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
    BOOST_LOG_TRIVIAL(info) << "Removing session " << req.name;

    m_sessions.UnloadByName(req.name);

    porla::Data::Models::Sessions::Remove(m_db, req.name);

    BOOST_LOG_TRIVIAL(info) << "Session " << req.name << " removed";

    cb.Ok(SessionsRemoveRes{});
}
