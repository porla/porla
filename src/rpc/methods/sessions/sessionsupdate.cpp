#include "sessionsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../../sessions.hpp"
#include "../../../data/models/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsUpdate;
using porla::Rpc::Methods::Sessions::SessionsUpdateReq;
using porla::Rpc::Methods::Sessions::SessionsUpdateRes;

SessionsUpdate::SessionsUpdate(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsUpdate::Execute(const SessionsUpdateReq& req, ResponseWriterHandle cb)
{
    auto session = Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    if (req.is_default && !session->is_default)
    {
        // If we send is_default=true and this session is not the default,
        // then make this session the default.

        porla::Data::Models::Sessions::SetDefault(
            m_db,
            req.id);

        BOOST_LOG_TRIVIAL(info) << "Default session is now '" << req.id << "'";
    }

    session->is_default = req.is_default;
    session->metadata   = req.metadata;
    session->name       = req.name;

    porla::Data::Models::Sessions::Update(
        m_db,
        *session);

    if (const auto& state = m_sessions.Get(session->id))
    {
        state->name = req.name;
    }

    cb->Ok(SessionsUpdateRes{});
}
