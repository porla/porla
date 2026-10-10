#include "sessionsget.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"

using porla::Rpc::Methods::Sessions::SessionsGet;
using porla::Rpc::Methods::Sessions::SessionsGetReq;
using porla::Rpc::Methods::Sessions::SessionsGetRes;

SessionsGet::SessionsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsGet::Execute(const SessionsGetReq &req, ResponseWriterHandle cb)
{
    const auto& session = m_sessions.Find(req.id);

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
                    .is_listening   = state->Libtorrent().is_listening(),
                    .is_paused      = state->Libtorrent().is_paused(),
                    .torrents_total = static_cast<int>(state->Count())
                }
        }
    });
}
