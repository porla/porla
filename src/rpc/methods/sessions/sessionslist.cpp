#include "sessionslist.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Data::Models::Sessions;
using porla::Rpc::Methods::Sessions::SessionsList;
using porla::Rpc::Methods::Sessions::SessionsListReq;
using porla::Rpc::Methods::Sessions::SessionsListRes;

SessionsList::SessionsList(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsList::Execute(const SessionsListReq& req, ResponseWriterHandle cb)
{
    const auto& sessions = Data::Models::Sessions::List(m_db);

    std::vector<SessionsListRes::Session> session_items;

    for (const auto& session : sessions)
    {
        const auto state = m_sessions.Get(session.id);

        session_items.push_back(SessionsListRes::Session{
            .id         = session.id,
            .name       = session.name,
            .is_default = session.is_default,
            .metadata   = session.metadata,
            .state      = state == nullptr
                ? std::optional<SessionsListRes::SessionState>()
                : SessionsListRes::SessionState{
                    .is_listening   = state->session->is_listening(),
                    .is_paused      = state->session->is_paused(),
                    .torrents_total = static_cast<int>(state->torrents.size())
                }
        });
    }

    cb->Ok(SessionsListRes{
        .sessions = session_items
    });
}
