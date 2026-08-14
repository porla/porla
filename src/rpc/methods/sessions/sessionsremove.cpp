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
    const auto& session = m_sessions.Get(req.id);

    if (session == nullptr)
    {
        return cb->Error(-1, "Session not found");
    }

    if (session->name == "default")
    {
        return cb->Error(-2, "Cannot remove default session");
    }

    m_sessions.UnloadById(session->id);

    porla::Data::Models::Sessions::Remove(m_db, session->id);

    BOOST_LOG_TRIVIAL(info) << "Session " << session->name << " removed";

    cb->Ok(SessionsRemoveRes{});
}
