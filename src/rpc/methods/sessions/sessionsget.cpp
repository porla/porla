#include "sessionsget.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsGet;
using porla::Rpc::Methods::Sessions::SessionsGetReq;
using porla::Rpc::Methods::Sessions::SessionsGetRes;

SessionsGet::SessionsGet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsGet::Execute(const SessionsGetReq &req, ResponseWriterHandle cb)
{
    const auto& session = Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& state = m_sessions.Get(req.id);

    cb->Ok(SessionsGetRes{
        .session = SessionsGetRes::Session{
            .id         = session->id,
            .name       = session->name,
            .is_default = session->is_default,
            .metadata   = session->metadata,
            .state      = state == nullptr
                ? std::optional<SessionsGetRes::SessionState>()
                : SessionsGetRes::SessionState{
                    .is_listening   = state->session->is_listening(),
                    .is_paused      = state->session->is_paused(),
                    .torrents_total = static_cast<int>(state->torrents.size())
                }
        }
    });
}
