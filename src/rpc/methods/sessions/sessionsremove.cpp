#include "sessionsremove.hpp"

#include <boost/log/trivial.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsRemove;
using porla::Rpc::Methods::Sessions::SessionsRemoveReq;
using porla::Rpc::Methods::Sessions::SessionsRemoveRes;

SessionsRemove::SessionsRemove(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsRemove::Execute(const SessionsRemoveReq& req, ResponseWriterHandle cb)
{
    const auto session = porla::Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    if (session->is_default)
    {
        return cb->Error(-2, "Cannot remove default session");
    }

    const auto session_state = m_sessions.Get(session->id);

    if (session_state != nullptr && session_state->torrents.size() > 0)
    {
        return cb->Error(-3, "Cannot remove session with torrents");
    }

    m_sessions.UnloadById(session->id);

    porla::Data::Models::Sessions::Remove(m_db, session->id);

    BOOST_LOG_TRIVIAL(info) << "Session " << session->name << " removed";

    cb->Ok(SessionsRemoveRes{});
}
